/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%        CCCC    OOO   M   M  PPPP    OOO   SSSSS  IIIII  TTTTT  EEEEE        %
%       C       O   O  MM MM  P   P  O   O  SS       I      T    E            %
%       C       O   O  M M M  PPPP   O   O   SSS     I      T    EEE          %
%       C       O   O  M   M  P      O   O     SS    I      T    E            %
%        CCCC    OOO   M   M  P       OOO   SSSSS  IIIII    T    EEEEE        %
%                                                                             %
%                                                                             %
%                      MagickWand Image Composite Methods                     %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2019 ImageMagick Studio LLC, a non-profit organization      %
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
%  Use the composite program to overlap one image over another.
%
*/

/*
  Include declarations.
*/
#include "MagickWand/studio.h"
#include "MagickWand/MagickWand.h"
#include "MagickWand/mogrify-private.h"
#include "MagickCore/composite-private.h"
#include "MagickCore/string-private.h"

/*
  Typedef declarations.
*/
typedef struct _CompositeOptions
{
  ChannelType
    channel;

  char
    *compose_args,
    *geometry;

  CompositeOperator
    compose;

  GravityType
    gravity;

  ssize_t
    stegano;

  RectangleInfo
    offset;

  MagickBooleanType
    clip_to_self,
    stereo,
    tile;
} CompositeOptions;

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  C o m p o s i t e I m a g e C o m m a n d                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CompositeImageCommand() reads one or more images and an optional mask and
%  composites them into a new image.
%
%  The format of the CompositeImageCommand method is:
%
%      MagickBooleanType CompositeImageCommand(ImageInfo *image_info,int argc,
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

static MagickBooleanType CompositeImageList(ImageInfo *image_info,Image **image,
  Image *composite_image,CompositeOptions *composite_options,
  ExceptionInfo *exception)
{
  const char
    *value;

  MagickStatusType
    status;

  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image **) NULL);
  assert((*image)->signature == MagickCoreSignature);
  if ((*image)->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",(*image)->filename);
  assert(exception != (ExceptionInfo *) NULL);
  (void) image_info;
  status=MagickTrue;
  composite_options->clip_to_self=GetCompositeClipToSelf(
    composite_options->compose);
  value=GetImageOption(image_info,"compose:clip-to-self");
  if (value != (const char *) NULL)
    composite_options->clip_to_self=IsStringTrue(value);
  value=GetImageOption(image_info,"compose:outside-overlay");
  if (value != (const char *) NULL)
    composite_options->clip_to_self=IsStringFalse(value);  /* deprecated */
  if (composite_image != (Image *) NULL)
    {
      ChannelType
        channel_mask;

      channel_mask=SetImageChannelMask(composite_image,
        composite_options->channel);
      assert(composite_image->signature == MagickCoreSignature);
      switch (composite_options->compose)
      {
        case BlendCompositeOp:
        case BlurCompositeOp:
        case DisplaceCompositeOp:
        case DistortCompositeOp:
        case DissolveCompositeOp:
        case ModulateCompositeOp:
        case ThresholdCompositeOp:
        {
          (void) SetImageArtifact(*image,"compose:args",
            composite_options->compose_args);
          break;
        }
        default:
          break;
      }
      /*
        Composite image.
      */
      if (composite_options->stegano != 0)
        {
          Image
            *stegano_image;

          (*image)->offset=composite_options->stegano-1;
          stegano_image=SteganoImage(*image,composite_image,exception);
          if (stegano_image != (Image *) NULL)
            {
              *image=DestroyImageList(*image);
              *image=stegano_image;
            }
        }
      else
        if (composite_options->stereo != MagickFalse)
          {
            Image
              *stereo_image;

            stereo_image=StereoAnaglyphImage(*image,composite_image,
              composite_options->offset.x,composite_options->offset.y,
              exception);
            if (stereo_image != (Image *) NULL)
              {
                *image=DestroyImageList(*image);
                *image=stereo_image;
              }
          }
        else
          if (composite_options->tile != MagickFalse)
            {
              size_t
                columns;

              ssize_t
                x,
                y;

              /*
                Tile the composite image.
              */
              columns=composite_image->columns;
              for (y=0; y < (ssize_t) (*image)->rows; y+=(ssize_t) composite_image->rows)
                for (x=0; x < (ssize_t) (*image)->columns; x+=(ssize_t) columns)
                  status&=CompositeImage(*image,composite_image,
                    composite_options->compose,MagickTrue,x,y,exception);
            }
          else
            {
              RectangleInfo
                geometry;

              /*
                Work out gravity Adjustment of Offset
              */
              SetGeometry(*image,&geometry);
              (void) ParseAbsoluteGeometry(composite_options->geometry,
                &geometry);
              geometry.width=composite_image->columns;
              geometry.height=composite_image->rows;
              GravityAdjustGeometry((*image)->columns,(*image)->rows,
                composite_options->gravity, &geometry);
              (*image)->gravity=(GravityType) composite_options->gravity;
              /*
                Digitally composite image.
              */
              status&=CompositeImage(*image,composite_image,
                composite_options->compose,composite_options->clip_to_self,
                geometry.x,geometry.y,exception);
            }
      (void) SetPixelChannelMask(composite_image,channel_mask);
    }
  return(status != 0 ? MagickTrue : MagickFalse);
}

static MagickBooleanType CompositeUsage(void)
{
  static const char
    miscellaneous[] =
      "  -debug events        display copious debugging information\n"
      "  -help                print program options\n"
      "  -list type           print a list of supported option arguments\n"
      "  -log format          format of debugging information\n"
      "  -version             print version information",
    operators[] =
      "  -blend geometry      blend images\n"
      "  -border geometry     surround image with a border of color\n"
      "  -bordercolor color   border color\n"
      "  -channel mask        set the image channel mask\n"
      "  -colors value        preferred number of colors in the image\n"
      "  -decipher filename    convert cipher pixels to plain pixels\n"
      "  -displace geometry   shift lookup according to a relative displacement map\n"
      "  -dissolve value      dissolve the two images a given percent\n"
      "  -distort geometry    shift lookup according to a absolute distortion map\n"
      "  -encipher filename   convert plain pixels to cipher pixels\n"
      "  -extract geometry    extract area from image\n"
      "  -geometry geometry   location of the composite image\n"
      "  -identify            identify the format and characteristics of the image\n"
      "  -monochrome          transform image to black and white\n"
      "  -negate              replace every pixel with its complementary color \n"
      "  -profile filename    add ICM or IPTC information profile to image\n"
      "  -quantize colorspace reduce colors in this colorspace\n"
      "  -repage geometry     size and location of an image canvas (operator)\n"
      "  -rotate degrees      apply Paeth rotation to the image\n"
      "  -resize geometry     resize the image\n"
      "  -sharpen geometry    sharpen the image\n"
      "  -shave geometry      shave pixels from the image edges\n"
      "  -stegano offset      hide watermark within an image\n"
      "  -stereo geometry     combine two image to create a stereo anaglyph\n"
      "  -strip               strip image of all profiles and comments\n"
      "  -thumbnail geometry  create a thumbnail of the image\n"
      "  -transform           affine transform image\n"
      "  -type type           image type\n"
      "  -unsharp geometry    sharpen the image\n"
      "  -watermark geometry  percent brightness and saturation of a watermark\n"
      "  -write filename      write images to this file",
    settings[] =
      "  -affine matrix       affine transform matrix\n"
      "  -alpha option        on, activate, off, deactivate, set, opaque, copy\n"
      "                       transparent, extract, background, or shape\n"
      "  -authenticate password\n"
      "                       decipher image with this password\n"
      "  -blue-primary point  chromaticity blue primary point\n"
      "  -colorspace type     alternate image colorspace\n"
      "  -comment string      annotate image with comment\n"
      "  -compose operator    composite operator\n"
      "  -compress type       type of pixel compression when writing the image\n"
      "  -define format:option\n"
      "                       define one or more image format options\n"
      "  -depth value         image depth\n"
      "  -density geometry    horizontal and vertical density of the image\n"
      "  -display server      get image or font from this X server\n"
      "  -dispose method      layer disposal method\n"
      "  -dither method       apply error diffusion to image\n"
      "  -encoding type       text encoding type\n"
      "  -endian type         endianness (MSB or LSB) of the image\n"
      "  -filter type         use this filter when resizing an image\n"
      "  -font name           render text with this font\n"
      "  -format \"string\"     output formatted image characteristics\n"
      "  -gravity type        which direction to gravitate towards\n"
      "  -green-primary point chromaticity green primary point\n"
      "  -interlace type      type of image interlacing scheme\n"
      "  -interpolate method  pixel color interpolation method\n"
      "  -label string        assign a label to an image\n"
      "  -limit type value    pixel cache resource limit\n"
      "  -matte               store matte channel if the image has one\n"
      "  -monitor             monitor progress\n"
      "  -page geometry       size and location of an image canvas (setting)\n"
      "  -pointsize value     font point size\n"
      "  -quality value       JPEG/MIFF/PNG compression level\n"
      "  -quiet               suppress all warning messages\n"
      "  -red-primary point   chromaticity red primary point\n"
      "  -regard-warnings     pay attention to warning messages\n"
      "  -respect-parentheses settings remain in effect until parenthesis boundary\n"
      "  -sampling-factor geometry\n"
      "                       horizontal and vertical sampling factor\n"
      "  -scene value         image scene number\n"
      "  -seed value          seed a new sequence of pseudo-random numbers\n"
      "  -size geometry       width and height of image\n"
      "  -support factor      resize support: > 1.0 is blurry, < 1.0 is sharp\n"
      "  -synchronize         synchronize image to storage device\n"
      "  -taint               declare the image as modified\n"
      "  -transparent-color color\n"
      "                       transparent color\n"
      "  -treedepth value     color tree depth\n"
      "  -tile                repeat composite operation across and down image\n"
      "  -units type          the units of image resolution\n"
      "  -verbose             print detailed information about the image\n"
      "  -virtual-pixel method\n"
      "                       virtual pixel access method\n"
      "  -white-point point   chromaticity white point",
    stack_operators[] =
      "  -swap indexes        swap two images in the image sequence";

  ListMagickVersion(stdout);
  (void) printf("Usage: %s [options ...] image [options ...] composite\n"
    "  [ [options ...] mask ] [options ...] composite\n",
    GetClientName());
  (void) printf("\nImage Settings:\n");
  (void) puts(settings);
  (void) printf("\nImage Operators:\n");
  (void) puts(operators);
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
  return(MagickFalse);
}

static void GetCompositeOptions(const ImageInfo *image_info,
  CompositeOptions *composite_options)
{
  (void) image_info;
  (void) memset(composite_options,0,sizeof(*composite_options));
  composite_options->channel=DefaultChannels;
  composite_options->compose=OverCompositeOp;
}

static void RelinquishCompositeOptions(CompositeOptions *composite_options)
{
  if (composite_options->compose_args != (char *) NULL)
    composite_options->compose_args=(char *)
      RelinquishMagickMemory(composite_options->compose_args);
  if (composite_options->geometry != (char *) NULL)
    composite_options->geometry=(char *)
      RelinquishMagickMemory(composite_options->geometry);
}

WandExport MagickBooleanType CompositeImageCommand(ImageInfo *image_info,
  int argc,char **argv,char **metadata,ExceptionInfo *exception)
{
#define NotInitialized  (unsigned int) (~0)
#define DestroyComposite() \
{ \
  RelinquishCompositeOptions(&composite_options); \
  DestroyImageStack(); \
  for (i=0; i < (ssize_t) argc; i++) \
    argv[i]=DestroyString(argv[i]); \
  argv=(char **) RelinquishMagickMemory(argv); \
}
#define ThrowCompositeException(asperity,tag,option) \
{ \
  (void) ThrowMagickException(exception,GetMagickModule(),asperity,tag,"`%s'", \
     option == (char *) NULL ? GetExceptionMessage(errno) : option); \
  DestroyComposite(); \
  return(MagickFalse); \
}
#define ThrowCompositeInvalidArgumentException(option,argument) \
{ \
  (void) ThrowMagickException(exception,GetMagickModule(),OptionError, \
    "InvalidArgument","'%s': %s",option,argument); \
  DestroyComposite(); \
  return(MagickFalse); \
}

  char
    *filename,
    *option;

  CompositeOptions
    composite_options;

  const char
    *format;

  Image
    *composite_image,
    *image,
    *images,
    *mask_image;

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
    Set default.
  */
  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
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
          return(MagickTrue);
        }
    }
  if (argc < 4)
    return(CompositeUsage());
  GetCompositeOptions(image_info,&composite_options);
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
    Check command syntax.
  */
  composite_image=NewImageList();
  image=NewImageList();
  mask_image=NewImageList();
  ReadCommandlLine(argc,&argv);
  status=ExpandFilenames(&argc,&argv);
  if (status == MagickFalse)
    ThrowCompositeException(ResourceLimitError,"MemoryAllocationFailed",
      GetExceptionMessage(errno));
  for (i=1; i < (ssize_t) (argc-1); i++)
  {
    option=argv[i];
    if (LocaleCompare(option,"(") == 0)
      {
        FireImageStack(MagickFalse,MagickTrue,pend);
        if (k == MaxImageStackDepth)
          ThrowCompositeException(OptionError,"ParenthesisNestedTooDeeply",
            option);
        PushImageStack();
        continue;
      }
    if (LocaleCompare(option,")") == 0)
      {
        FireImageStack(MagickFalse,MagickTrue,MagickTrue);
        if (k == 0)
          ThrowCompositeException(OptionError,"UnableToParseExpression",option);
        PopImageStack();
        continue;
      }
    if (IsCommandOption(option) == MagickFalse)
      {
        /*
          Read input image.
        */
        FireImageStack(MagickFalse,MagickFalse,pend);
        filename=argv[i];
        if ((LocaleCompare(filename,"--") == 0) && (i < (ssize_t) (argc-1)))
          filename=argv[++i];
        images=ReadImages(image_info,filename,exception);
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
        if (LocaleCompare("affine",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
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
              ThrowCompositeException(OptionError,"MissingArgument",option);
            type=ParseCommandOption(MagickAlphaChannelOptions,MagickFalse,argv[i]);
            if (type < 0)
              ThrowCompositeException(OptionError,
                "UnrecognizedAlphaChannelOption",argv[i]);
            break;
          }
        if (LocaleCompare("authenticate",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            break;
          }
        ThrowCompositeException(OptionError,"UnrecognizedOption",option)
      }
      case 'b':
      {
        if (LocaleCompare("background",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("blend",option+1) == 0)
          {
            (void) CloneString(&composite_options.compose_args,(char *) NULL);
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            (void) CloneString(&composite_options.compose_args,argv[i]);
            composite_options.compose=BlendCompositeOp;
            break;
          }
        if (LocaleCompare("blur",option+1) == 0)
          {
            (void) CloneString(&composite_options.compose_args,(char *) NULL);
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            (void) CloneString(&composite_options.compose_args,argv[i]);
            composite_options.compose=BlurCompositeOp;
            break;
          }
        if (LocaleCompare("blue-primary",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("border",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("bordercolor",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            break;
          }
        ThrowCompositeException(OptionError,"UnrecognizedOption",option)
      }
      case 'c':
      {
        if (LocaleCompare("cache",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("channel",option+1) == 0)
          {
            ssize_t
              channel;

            if (*option == '+')
              {
                composite_options.channel=DefaultChannels;
                break;
              }
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            channel=ParseChannelOption(argv[i]);
            if (channel < 0)
              ThrowCompositeException(OptionError,"UnrecognizedChannelType",
                argv[i]);
            composite_options.channel=(ChannelType) channel;
            break;
          }
        if (LocaleCompare("colors",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
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
              ThrowCompositeException(OptionError,"MissingArgument",option);
            colorspace=ParseCommandOption(MagickColorspaceOptions,
              MagickFalse,argv[i]);
            if (colorspace < 0)
              ThrowCompositeException(OptionError,"UnrecognizedColorspace",
                argv[i]);
            break;
          }
        if (LocaleCompare("comment",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("compose",option+1) == 0)
          {
            ssize_t
              compose;

            composite_options.compose=UndefinedCompositeOp;
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            compose=ParseCommandOption(MagickComposeOptions,MagickFalse,
              argv[i]);
            if (compose < 0)
              ThrowCompositeException(OptionError,"UnrecognizedComposeOperator",
                argv[i]);
            composite_options.compose=(CompositeOperator) compose;
            break;
          }
        if (LocaleCompare("compress",option+1) == 0)
          {
            ssize_t
              compress;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            compress=ParseCommandOption(MagickCompressOptions,MagickFalse,
              argv[i]);
            if (compress < 0)
              ThrowCompositeException(OptionError,
                "UnrecognizedImageCompression",argv[i]);
            break;
          }
        if (LocaleCompare("concurrent",option+1) == 0)
          break;
        ThrowCompositeException(OptionError,"UnrecognizedOption",option)
      }
      case 'd':
      {
        if (LocaleCompare("debug",option+1) == 0)
          {
            ssize_t
              event;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            event=ParseCommandOption(MagickLogEventOptions,MagickFalse,argv[i]);
            if (event < 0)
              ThrowCompositeException(OptionError,"UnrecognizedEventType",
                argv[i]);
            (void) SetLogEventMask(argv[i]);
            break;
          }
        if (LocaleCompare("decipher",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("define",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (*option == '+')
              {
                const char
                  *define;

                define=GetImageOption(image_info,argv[i]);
                if (define == (const char *) NULL)
                  ThrowCompositeException(OptionError,"NoSuchOption",argv[i]);
                break;
              }
            break;
          }
        if (LocaleCompare("density",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("depth",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("displace",option+1) == 0)
          {
            (void) CloneString(&composite_options.compose_args,(char *) NULL);
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            (void) CloneString(&composite_options.compose_args,argv[i]);
            composite_options.compose=DisplaceCompositeOp;
            break;
          }
        if (LocaleCompare("display",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
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
              ThrowCompositeException(OptionError,"MissingArgument",option);
            dispose=ParseCommandOption(MagickDisposeOptions,MagickFalse,argv[i]);
            if (dispose < 0)
              ThrowCompositeException(OptionError,"UnrecognizedDisposeMethod",
                argv[i]);
            break;
          }
        if (LocaleCompare("dissolve",option+1) == 0)
          {
            (void) CloneString(&composite_options.compose_args,(char *) NULL);
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            (void) CloneString(&composite_options.compose_args,argv[i]);
            composite_options.compose=DissolveCompositeOp;
            break;
          }
        if (LocaleCompare("distort",option+1) == 0)
          {
            (void) CloneString(&composite_options.compose_args,(char *) NULL);
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            (void) CloneString(&composite_options.compose_args,argv[i]);
            composite_options.compose=DistortCompositeOp;
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
              ThrowCompositeException(OptionError,"MissingArgument",option);
            method=ParseCommandOption(MagickDitherOptions,MagickFalse,argv[i]);
            if (method < 0)
              ThrowCompositeException(OptionError,"UnrecognizedDitherMethod",
                argv[i]);
            break;
          }
        if (LocaleCompare("duration",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowCompositeException(OptionError,"UnrecognizedOption",option)
      }
      case 'e':
      {
        if (LocaleCompare("encipher",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("encoding",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
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
              ThrowCompositeException(OptionError,"MissingArgument",option);
            endian=ParseCommandOption(MagickEndianOptions,MagickFalse,
              argv[i]);
            if (endian < 0)
              ThrowCompositeException(OptionError,"UnrecognizedEndianType",
                argv[i]);
            break;
          }
        if (LocaleCompare("extract",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowCompositeException(OptionError,"UnrecognizedOption",option)
      }
      case 'f':
      {
        if (LocaleCompare("filter",option+1) == 0)
          {
            ssize_t
              filter;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            filter=ParseCommandOption(MagickFilterOptions,MagickFalse,argv[i]);
            if (filter < 0)
              ThrowCompositeException(OptionError,"UnrecognizedImageFilter",
                argv[i]);
            break;
          }
        if (LocaleCompare("font",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("format",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            format=argv[i];
            break;
          }
        ThrowCompositeException(OptionError,"UnrecognizedOption",option)
      }
      case 'g':
      {
        if (LocaleCompare("geometry",option+1) == 0)
          {
            (void) CloneString(&composite_options.geometry,(char *) NULL);
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            (void) CloneString(&composite_options.geometry,argv[i]);
            break;
          }
        if (LocaleCompare("gravity",option+1) == 0)
          {
            ssize_t
              gravity;

            composite_options.gravity=UndefinedGravity;
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            gravity=ParseCommandOption(MagickGravityOptions,MagickFalse,
              argv[i]);
            if (gravity < 0)
              ThrowCompositeException(OptionError,"UnrecognizedGravityType",
                argv[i]);
            composite_options.gravity=(GravityType) gravity;
            break;
          }
        if (LocaleCompare("green-primary",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowCompositeException(OptionError,"UnrecognizedOption",option)
      }
      case 'h':
      {
        if ((LocaleCompare("help",option+1) == 0) ||
            (LocaleCompare("-help",option+1) == 0))
          return(CompositeUsage());
        ThrowCompositeException(OptionError,"UnrecognizedOption",option)
      }
      case 'i':
      {
        if (LocaleCompare("identify",option+1) == 0)
          break;
        if (LocaleCompare("interlace",option+1) == 0)
          {
            ssize_t
              interlace;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            interlace=ParseCommandOption(MagickInterlaceOptions,MagickFalse,
              argv[i]);
            if (interlace < 0)
              ThrowCompositeException(OptionError,
                "UnrecognizedInterlaceType",argv[i]);
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
              ThrowCompositeException(OptionError,"MissingArgument",option);
            interpolate=ParseCommandOption(MagickInterpolateOptions,MagickFalse,
              argv[i]);
            if (interpolate < 0)
              ThrowCompositeException(OptionError,
                "UnrecognizedInterpolateMethod",argv[i]);
            break;
          }
        ThrowCompositeException(OptionError,"UnrecognizedOption",option)
      }
      case 'l':
      {
        if (LocaleCompare("label",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
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
              ThrowCompositeException(OptionError,"MissingArgument",option);
            resource=ParseCommandOption(MagickResourceOptions,MagickFalse,
              argv[i]);
            if (resource < 0)
              ThrowCompositeException(OptionError,"UnrecognizedResourceType",
                argv[i]);
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            value=StringToDouble(argv[i],&p);
            (void) value;
            if ((p == argv[i]) && (LocaleCompare("unlimited",argv[i]) != 0))
              ThrowCompositeInvalidArgumentException(option,argv[i]);
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
              ThrowCompositeException(OptionError,"MissingArgument",option);
            list=ParseCommandOption(MagickListOptions,MagickFalse,argv[i]);
            if (list < 0)
              ThrowCompositeException(OptionError,"UnrecognizedListType",
                argv[i]);
            status=MogrifyImageInfo(image_info,(int) (i-j+1),(const char **)
              argv+j,exception);
            DestroyComposite();
            return(status == 0 ? MagickFalse : MagickTrue);
          }
        if (LocaleCompare("log",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if ((i == (ssize_t) argc) || (strchr(argv[i],'%') == (char *) NULL))
              ThrowCompositeException(OptionError,"MissingArgument",option);
            break;
          }
        ThrowCompositeException(OptionError,"UnrecognizedOption",option)
      }
      case 'm':
      {
        if (LocaleCompare("matte",option+1) == 0)
          break;
        if (LocaleCompare("monitor",option+1) == 0)
          break;
        if (LocaleCompare("monochrome",option+1) == 0)
          break;
        ThrowCompositeException(OptionError,"UnrecognizedOption",option)
      }
      case 'n':
      {
        if (LocaleCompare("negate",option+1) == 0)
          break;
        if (LocaleCompare("noop",option+1) == 0)
          break;
        ThrowCompositeException(OptionError,"UnrecognizedOption",option)
      }
      case 'p':
      {
        if (LocaleCompare("page",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("pointsize",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("process",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("profile",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            break;
          }
        ThrowCompositeException(OptionError,"UnrecognizedOption",option)
      }
      case 'q':
      {
        if (LocaleCompare("quality",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
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
              ThrowCompositeException(OptionError,"MissingArgument",option);
            colorspace=ParseCommandOption(MagickColorspaceOptions,
              MagickFalse,argv[i]);
            if (colorspace < 0)
              ThrowCompositeException(OptionError,"UnrecognizedColorspace",
                argv[i]);
            break;
          }
        if (LocaleCompare("quiet",option+1) == 0)
          break;
        ThrowCompositeException(OptionError,"UnrecognizedOption",option)
      }
      case 'r':
      {
        if (LocaleCompare("red-primary",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("regard-warnings",option+1) == 0)
          break;
        if (LocaleCompare("render",option+1) == 0)
          break;
        if (LocaleCompare("repage",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleNCompare("respect-parentheses",option+1,17) == 0)
          {
            respect_parenthesis=(*option == '-') ? MagickTrue : MagickFalse;
            break;
          }
        if (LocaleCompare("resize",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("rotate",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowCompositeException(OptionError,"UnrecognizedOption",option)
      }
      case 's':
      {
        if (LocaleCompare("sampling-factor",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("scene",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("seed",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("sharpen",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("shave",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("size",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("stegano",option+1) == 0)
          {
            composite_options.stegano=0;
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            composite_options.stegano=(ssize_t) StringToLong(argv[i])+1;
            break;
          }
        if (LocaleCompare("stereo",option+1) == 0)
          {
            MagickStatusType
              flags;

            composite_options.stereo=MagickFalse;
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            flags=ParseAbsoluteGeometry(argv[i],&composite_options.offset);
            if ((flags & YValue) == 0)
              composite_options.offset.y=composite_options.offset.x;
            composite_options.stereo=MagickTrue;
            break;
          }
        if (LocaleCompare("strip",option+1) == 0)
          break;
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
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("synchronize",option+1) == 0)
          break;
        ThrowCompositeException(OptionError,"UnrecognizedOption",option)
      }
      case 't':
      {
        if (LocaleCompare("taint",option+1) == 0)
          break;
        if (LocaleCompare("thumbnail",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("tile",option+1) == 0)
          {
            composite_options.tile=(*option == '-') ? MagickTrue : MagickFalse;
            (void) CopyMagickString(argv[i]+1,"sans",MagickPathExtent);
            break;
          }
        if (LocaleCompare("transform",option+1) == 0)
          break;
        if (LocaleCompare("transparent-color",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("treedepth",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("type",option+1) == 0)
          {
            ssize_t
              type;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            type=ParseCommandOption(MagickTypeOptions,MagickFalse,argv[i]);
            if (type < 0)
              ThrowCompositeException(OptionError,"UnrecognizedImageType",
                argv[i]);
            break;
          }
        ThrowCompositeException(OptionError,"UnrecognizedOption",option)
      }
      case 'u':
      {
        if (LocaleCompare("units",option+1) == 0)
          {
            ssize_t
              units;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            units=ParseCommandOption(MagickResolutionOptions,MagickFalse,
              argv[i]);
            if (units < 0)
              ThrowCompositeException(OptionError,"UnrecognizedUnitsType",
                argv[i]);
            break;
          }
        if (LocaleCompare("unsharp",option+1) == 0)
          {
            (void) CloneString(&composite_options.compose_args,(char *) NULL);
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            (void) CloneString(&composite_options.compose_args,argv[i]);
            composite_options.compose=ThresholdCompositeOp;
            break;
          }
        ThrowCompositeException(OptionError,"UnrecognizedOption",option)
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
        if (LocaleCompare("virtual-pixel",option+1) == 0)
          {
            ssize_t
              method;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            method=ParseCommandOption(MagickVirtualPixelOptions,MagickFalse,
              argv[i]);
            if (method < 0)
              ThrowCompositeException(OptionError,
                "UnrecognizedVirtualPixelMethod",argv[i]);
            break;
          }
        ThrowCompositeException(OptionError,"UnrecognizedOption",option)
      }
      case 'w':
      {
        if (LocaleCompare("watermark",option+1) == 0)
          {
            (void) CloneString(&composite_options.compose_args,(char *) NULL);
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            (void) CloneString(&composite_options.compose_args,argv[i]);
            composite_options.compose=ModulateCompositeOp;
            break;
          }
        if (LocaleCompare("white-point",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("write",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            break;
          }
        ThrowCompositeException(OptionError,"UnrecognizedOption",option)
      }
      case '?':
        break;
      default:
        ThrowCompositeException(OptionError,"UnrecognizedOption",option)
    }
    fire=(GetCommandOptionFlags(MagickCommandOptions,MagickFalse,option) &
      FireOptionFlag) == 0 ?  MagickFalse : MagickTrue;
    if (fire != MagickFalse)
      FireImageStack(MagickFalse,MagickTrue,MagickTrue);
  }
  if (k != 0)
    ThrowCompositeException(OptionError,"UnbalancedParenthesis",argv[i]);
  if (i-- != (ssize_t) (argc-1))
    ThrowCompositeException(OptionError,"MissingAnImageFilename",argv[i]);
  if ((image == (Image *) NULL) || (GetImageListLength(image) < 2))
    ThrowCompositeException(OptionError,"MissingAnImageFilename",argv[argc-1]);
  FinalizeImageSettings(image_info,image,MagickTrue);
  if ((image == (Image *) NULL) || (GetImageListLength(image) < 2))
    ThrowCompositeException(OptionError,"MissingAnImageFilename",argv[argc-1]);
  /*
    Composite images.
  */
  RemoveImageStack(composite_image);
  RemoveImageStack(images);
  if (composite_image->geometry != (char *) NULL)
    {
      RectangleInfo
        resize_geometry;

      (void) ParseRegionGeometry(composite_image,composite_image->geometry,
        &resize_geometry,exception);
      if ((composite_image->columns != resize_geometry.width) ||
          (composite_image->rows != resize_geometry.height))
        {
          Image
            *resize_image;

          resize_image=ResizeImage(composite_image,resize_geometry.width,
            resize_geometry.height,composite_image->filter,exception);
          if (resize_image != (Image *) NULL)
            {
              composite_image=DestroyImage(composite_image);
              composite_image=resize_image;
            }
        }
    }
  RemoveImageStack(mask_image);
  if (mask_image != (Image *) NULL)
    {
      if ((composite_options.compose == DisplaceCompositeOp) ||
          (composite_options.compose == DistortCompositeOp))
        status&=CompositeImage(composite_image,mask_image,
          CopyGreenCompositeOp,MagickTrue,0,0,exception);
      else
        status&=CompositeImage(composite_image,mask_image,IntensityCompositeOp,
          MagickTrue,0,0,exception);
      mask_image=DestroyImage(mask_image);
    }
  status&=CompositeImageList(image_info,&images,composite_image,
    &composite_options,exception);
  composite_image=DestroyImage(composite_image);
  /*
    Write composite images.
  */
  status&=WriteImages(image_info,images,argv[argc-1],exception);
  if (metadata != (char **) NULL)
    {
      char
        *text;

      text=InterpretImageProperties(image_info,images,format,exception);
      if (text == (char *) NULL)
        ThrowCompositeException(ResourceLimitError,"MemoryAllocationFailed",
          GetExceptionMessage(errno));
      (void) ConcatenateString(&(*metadata),text);
      text=DestroyString(text);
    }
  images=DestroyImageList(images);
  RelinquishCompositeOptions(&composite_options);
  DestroyComposite();
  return(status != 0 ? MagickTrue : MagickFalse);
}
