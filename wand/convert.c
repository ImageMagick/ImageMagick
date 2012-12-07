/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                CCCC   OOO   N   N  V   V  EEEEE  RRRR   TTTTT               %
%               C      O   O  NN  N  V   V  E      R   R    T                 %
%               C      O   O  N N N  V   V  EEE    RRRR     T                 %
%               C      O   O  N  NN   V V   E      R R      T                 %
%                CCCC   OOO   N   N    V    EEEEE  R  R     T                 %
%                                                                             %
%                                                                             %
%                Convert an image from one format to another.                 %
%                                                                             %
%                              Software Design                                %
%                                John Cristy                                  %
%                                April 1992                                   %
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
%  See the License for the specific language governing permissions and        %
%  limitations under the License.                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Use the convert program to convert between image formats as well as resize
%  an image, blur, crop, despeckle, dither, draw on, flip, join, re-sample,
%  and much more.
%
*/

/*
  Include declarations.
*/
#include "wand/studio.h"
#include "wand/MagickWand.h"
#include "wand/mogrify-private.h"
#include "magick/string-private.h"
#include "magick/utility-private.h"

/*
  Define declarations.
*/
#define ThrowFileException(exception,severity,tag,context) \
{ \
  (void) ThrowMagickException(exception,GetMagickModule(),severity, \
    tag == (const char *) NULL ? "unknown" : tag,"`%s': %s",context, \
    GetExceptionMessage(errno)); \
}

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

  register ssize_t
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
  for (i=2; i < (ssize_t) (argc-1); i++)
  {
    input=fopen_utf8(argv[i],"rb");
    if (input == (FILE *) NULL)
      {
        ThrowFileException(exception,FileOpenError,"UnableToOpenFile",argv[i]);
        continue;
      }
    for (c=fgetc(input); c != EOF; c=fgetc(input))
      (void) fputc((char) c,output);
    (void) fclose(input);
    (void) remove_utf8(argv[i]);
  }
  (void) fclose(output);
  return(MagickTrue);
}

static MagickBooleanType ConvertUsage(void)
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
      "-blue-shift factor   simulate a scene at nighttime in the moonlight",
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
      "-identify            identify the format and characteristics of the image",
      "-ift                 implements the inverse discrete Fourier transform (DFT)",
      "-implode amount      implode image pixels about the center",
      "-interpolative-resize geometry",
      "                     resize image using 'point sampled' interpolation",
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
      "-statistic type geometry",
      "                     replace each pixel with corresponding statistic from the",
      "                     neighborhood",
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
      "-clone indexes       clone an image",
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
  (void) printf("Usage: %s [options ...] file [ [options ...] "
    "file ...] [options ...] file\n",GetClientName());
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
    "InvalidArgument","`%s': %s",option,argument); \
  DestroyConvert(); \
  return(MagickFalse); \
}

  char
    *filename,
    *option;

  const char
    *format;

  Image
    *image;

  ImageStack
    image_stack[MaxImageStackDepth+1];

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
          (void) FormatLocaleFile(stdout,"Version: %s\n",
            GetMagickVersion((size_t *) NULL));
          (void) FormatLocaleFile(stdout,"Copyright: %s\n",
            GetMagickCopyright());
          (void) FormatLocaleFile(stdout,"Features: %s\n\n",
            GetMagickFeatures());
          return(MagickFalse);
        }
    }
  if (argc < 3)
    return(ConvertUsage());
  filename=(char *) NULL;
  format="%w,%h,%m";
  j=1;
  k=0;
  NewImageStack();
  option=(char *) NULL;
  pend=MagickFalse;
  respect_parenthesis=MagickFalse;
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
        (void) SetImageOption(image_info,"filename",filename);
        (void) CopyMagickString(image_info->filename,filename,MaxTextExtent);
        if (image_info->ping != MagickFalse)
          images=PingImages(image_info,exception);
        else
          images=ReadImages(image_info,exception);
        status&=(images != (Image *) NULL) &&
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
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("adaptive-resize",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("adaptive-sharpen",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            type=ParseCommandOption(MagickAlphaOptions,MagickFalse,argv[i]);
            if (type < 0)
              ThrowConvertException(OptionError,"UnrecognizedAlphaChannelType",
                argv[i]);
            break;
          }
        if (LocaleCompare("annotate",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            i++;
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
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
        ThrowConvertException(OptionError,"UnrecognizedOption",option)
      }
      case 'b':
      {
        if (LocaleCompare("background",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("bench",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("box",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("brightness-contrast",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            channel=ParseChannelOption(argv[i]);
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("clip-path",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("clone",option+1) == 0)
          {
            Image
              *clone_images;

            clone_images=image;
            if (k != 0)
              clone_images=image_stack[k-1].image;
            if (clone_images == (Image *) NULL)
              ThrowConvertException(ImageError,"ImageSequenceRequired",option);
            FireImageStack(MagickTrue,MagickTrue,MagickTrue);
            if (*option == '+')
              clone_images=CloneImages(clone_images,"-1",exception);
            else
              {
                i++;
                if (i == (ssize_t) (argc-1))
                  ThrowConvertException(OptionError,"MissingArgument",option);
                if (IsSceneGeometry(argv[i],MagickFalse) == MagickFalse)
                  ThrowConvertInvalidArgumentException(option,argv[i]);
                clone_images=CloneImages(clone_images,argv[i],exception);
              }
            if (clone_images == (Image *) NULL)
              ThrowConvertException(OptionError,"NoSuchImage",option);
            AppendImageStack(clone_images);
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            kernel_info=AcquireKernelInfo(argv[i]);
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
            if ((i == (ssize_t) (argc-1)) ||
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
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            colorspace=ParseCommandOption(MagickColorspaceOptions,
              MagickFalse,argv[i]);
            if (colorspace < 0)
              ThrowConvertException(OptionError,"UnrecognizedColorspace",
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
              ThrowConvertException(OptionError,"MissingArgument",option);
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
            if (i == (ssize_t) (argc-1))
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
        if (LocaleCompare("contrast",option+1) == 0)
          break;
        if (LocaleCompare("contrast-stretch",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            kernel_info=AcquireKernelInfo(argv[i]);
            if (kernel_info == (KernelInfo *) NULL)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            kernel_info=DestroyKernelInfo(kernel_info);
            break;
          }
        if (LocaleCompare("crop",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
        if (LocaleCompare("delete",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsSceneGeometry(argv[i],MagickFalse) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("density",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("deskew",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("duplicate",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("encoding",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("features",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("font",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("format",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            format=argv[i];
            break;
          }
        if (LocaleCompare("frame",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
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
             if (i == (ssize_t) (argc-1))
               ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("fuzz",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("fx",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if ((LocaleCompare("gaussian-blur",option+1) == 0) ||
            (LocaleCompare("gaussian",option+1) == 0))
          {
            i++;
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            gravity=ParseCommandOption(MagickGravityOptions,MagickFalse,
              argv[i]);
            if (gravity < 0)
              ThrowConvertException(OptionError,"UnrecognizedGravityType",
                argv[i]);
            break;
          }
        if (LocaleCompare("green-primary",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
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
          return(ConvertUsage());
        ThrowConvertException(OptionError,"UnrecognizedOption",option)
      }
      case 'i':
      {
        if (LocaleCompare("identify",option+1) == 0)
          break;
        if (LocaleCompare("ift",option+1) == 0)
          break;
        if (LocaleCompare("implode",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("interpolative-resize",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
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
              ThrowConvertException(OptionError,"MissingArgument",option);
            intent=ParseCommandOption(MagickIntentOptions,MagickFalse,
              argv[i]);
            if (intent < 0)
              ThrowConvertException(OptionError,"UnrecognizedIntentType",
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("level-colors",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("linewidth",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("liquid-rescale",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
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
            if (i == (ssize_t) (argc-1))
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
            return(status != 0 ? MagickFalse : MagickTrue);
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
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowConvertException(OptionError,"UnrecognizedOption",option)
      }
      case 'm':
      {
        if (LocaleCompare("map",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("mask",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("maximum",option+1) == 0)
          break;
        if (LocaleCompare("median",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("minimum",option+1) == 0)
          break;
        if (LocaleCompare("mode",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
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
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
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
              ThrowConvertException(OptionError,"MissingArgument",option);
            GetMagickToken(argv[i],NULL,token);
            op=ParseCommandOption(MagickMorphologyOptions,MagickFalse,token);
            if (op < 0)
              ThrowConvertException(OptionError,"UnrecognizedMorphologyMethod",
                token);
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            kernel_info=AcquireKernelInfo(argv[i]);
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("ordered-dither",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("process",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("profile",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
        if (LocaleCompare("radial-blur",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("raise",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("recolor",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("red-primary",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
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
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("rotate",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("selective-blur",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("sharpen",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("shear",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("sigmoidal-contrast",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("smush",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
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
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("splice",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
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
            if ((i == (ssize_t) (argc-1)) ||
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("strokewidth",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("thumbnail",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("threshold",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("tile-offset",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("transparent-color",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            (void) FormatLocaleFile(stdout,"Version: %s\n",
              GetMagickVersion((size_t *) NULL));
            (void) FormatLocaleFile(stdout,"Copyright: %s\n",
              GetMagickCopyright());
            (void) FormatLocaleFile(stdout,"Features: %s\n\n",
              GetMagickFeatures());
            break;
          }
        if (LocaleCompare("view",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("vignette",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
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
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("white-point",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("white-threshold",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowConvertException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConvertInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("write",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) (argc-1))
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
  if (LocaleCompare(" ",argv[argc-1]) == 0) /* common line continuation error */
    ThrowConvertException(OptionError,"MissingAnImageFilename",argv[argc-1]);
  status&=WriteImages(image_info,image,argv[argc-1],exception);
  if (metadata != (char **) NULL)
    {
      char
        *text;

      text=InterpretImageProperties(image_info,image,format);
      if (text == (char *) NULL)
        ThrowConvertException(ResourceLimitError,"MemoryAllocationFailed",
          GetExceptionMessage(errno));
      (void) ConcatenateString(&(*metadata),text);
      text=DestroyString(text);
    }
  DestroyConvert();
  return(status != 0 ? MagickTrue : MagickFalse);
}
