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
%                                John Cristy                                  %
%                                 July 1992                                   %
%                                                                             %
%                                                                             %
%  Copyright 1999-2009 ImageMagick Studio LLC, a non-profit organization      %
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
%  Use the composite program to overlap one image over another.
%
*/

/*
  Include declarations.
*/
#include "wand/studio.h"
#include "wand/MagickWand.h"
#include "wand/mogrify-private.h"

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

  long
    stegano;

  RectangleInfo
    offset;

  MagickBooleanType
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
  MagickStatusType
    status;

  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image != (Image **) NULL);
  assert((*image)->signature == MagickSignature);
  if ((*image)->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",(*image)->filename);
  assert(exception != (ExceptionInfo *) NULL);
  status=MagickTrue;
  if (composite_image != (Image *) NULL)
    {
      assert(composite_image->signature == MagickSignature);
      switch( composite_options->compose )
        {
          case BlendCompositeOp:
          case BlurCompositeOp:
          case DisplaceCompositeOp:
          case DistortCompositeOp:
          case DissolveCompositeOp:
          case ModulateCompositeOp:
          case ThresholdCompositeOp:
            (void) SetImageArtifact(composite_image,"compose:args",
                        composite_options->compose_args);
            break;
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
              long
                x,
                y;

              unsigned long
                columns;

              /*
                Tile the composite image.
              */
              (void) SetImageArtifact(composite_image,"compose:outside-overlay",
                "false");
              columns=composite_image->columns;
              for (y=0; y < (long) (*image)->rows; y+=composite_image->rows)
                for (x=0; x < (long) (*image)->columns; x+=columns)
                  status&=CompositeImageChannel(*image,
                    composite_options->channel,composite_options->compose,
                    composite_image,x,y);
              GetImageException(*image,exception);
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
              status&=CompositeImageChannel(*image,composite_options->channel,
                composite_options->compose,composite_image,geometry.x,
                geometry.y);
              GetImageException(*image,exception);
            }
    }
  return(status != 0 ? MagickTrue : MagickFalse);
}

static MagickBooleanType CompositeUsage(void)
{
  const char
    **p;

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
      "-blend geometry      blend images",
      "-border geometry     surround image with a border of color",
      "-bordercolor color   border color",
      "-colors value        preferred number of colors in the image",
      "-decipher filename    convert cipher pixels to plain pixels",
      "-displace geometry   shift lookup according to a relative displacement map",
      "-dissolve value      dissolve the two images a given percent",
      "-distort geometry    shift lookup according to a absolute distortion map",
      "-encipher filename   convert plain pixels to cipher pixels",
      "-extract geometry    extract area from image",
      "-geometry geometry   location of the composite image",
      "-identify            identify the format and characteristics of the image",
      "-monochrome          transform image to black and white",
      "-negate              replace every pixel with its complementary color ",
      "-profile filename    add ICM or IPTC information profile to image",
      "-quantize colorspace reduce colors in this colorspace",
      "-repage geometry     size and location of an image canvas (operator)",
      "-rotate degrees      apply Paeth rotation to the image",
      "-resize geometry     resize the image",
      "-sharpen geometry    sharpen the image",
      "-shave geometry      shave pixels from the image edges",
      "-stegano offset      hide watermark within an image",
      "-stereo geometry     combine two image to create a stereo anaglyph",
      "-strip               strip image of all profiles and comments",
      "-thumbnail geometry  create a thumbnail of the image",
      "-transform           affine transform image",
      "-type type           image type",
      "-unsharp geometry    sharpen the image",
      "-watermark geometry  percent brightness and saturation of a watermark",
      "-write filename      write images to this file",
      (char *) NULL
    },
    *settings[]=
    {
      "-affine matrix       affine transform matrix",
      "-alpha option        on, activate, off, deactivate, set, opaque, copy",
      "                     transparent, extract, background, or shape",
      "-authenticate password",
      "                     decipher image with this password",
      "-blue-primary point  chromaticity blue primary point",
      "-channel type        apply option to select image channels",
      "-colorspace type     alternate image colorspace",
      "-comment string      annotate image with comment",
      "-compose operator    composite operator",
      "-compress type       type of pixel compression when writing the image",
      "-define format:option",
      "                     define one or more image format options",
      "-depth value         image depth",
      "-density geometry    horizontal and vertical density of the image",
      "-display server      get image or font from this X server",
      "-dispose method      layer disposal method",
      "-dither method       apply error diffusion to image",
      "-encoding type       text encoding type",
      "-endian type         endianness (MSB or LSB) of the image",
      "-filter type         use this filter when resizing an image",
      "-font name           render text with this font",
      "-format \"string\"     output formatted image characteristics",
      "-gravity type        which direction to gravitate towards",
      "-green-primary point chromaticity green primary point",
      "-interlace type      type of image interlacing scheme",
      "-interpolate method  pixel color interpolation method",
      "-label string        assign a label to an image",
      "-limit type value    pixel cache resource limit",
      "-monitor             monitor progress",
      "-page geometry       size and location of an image canvas (setting)",
      "-pointsize value     font point size",
      "-quality value       JPEG/MIFF/PNG compression level",
      "-quiet               suppress all warning messages",
      "-red-primary point   chromaticity red primary point",
      "-regard-warnings     pay attention to warning messages",
      "-respect-parentheses settings remain in effect until parenthesis boundary",
      "-sampling-factor geometry",
      "                     horizontal and vertical sampling factor",
      "-scene value         image scene number",
      "-seed value          seed a new sequence of pseudo-random numbers",
      "-size geometry       width and height of image",
      "-transparent-color color",
      "                     transparent color",
      "-treedepth value     color tree depth",
      "-tile                repeat composite operation across and down image",
      "-units type          the units of image resolution",
      "-verbose             print detailed information about the image",
      "-virtual-pixel method",
      "                     virtual pixel access method",
      "-white-point point   chromaticity white point",
      (char *) NULL
    },
    *stack_operators[]=
    {
      "-swap indexes        swap two images in the image sequence",
      (char *) NULL
    };


  (void) printf("Version: %s\n",GetMagickVersion((unsigned long *) NULL));
  (void) printf("Copyright: %s\n\n",GetMagickCopyright());
  (void) printf("Usage: %s [options ...] image [options ...] composite\n"
    "  [ [options ...] mask ] [options ...] composite\n",
    GetClientName());
  (void) printf("\nImage Settings:\n");
  for (p=settings; *p != (char *) NULL; p++)
    (void) printf("  %s\n",*p);
  (void) printf("\nImage Operators:\n");
  for (p=operators; *p != (char *) NULL; p++)
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

static void GetCompositeOptions(CompositeOptions *composite_options)
{
  (void) ResetMagickMemory(composite_options,0,sizeof(*composite_options));
  composite_options->channel=DefaultChannels;
  composite_options->compose=OverCompositeOp;
}

static void RelinquishCompositeOptions(CompositeOptions *composite_options)
{
  if (composite_options->compose_args != (char *) NULL)
    composite_options->compose_args=(char *)
      RelinquishMagickMemory(composite_options->compose_args);
}

WandExport MagickBooleanType CompositeImageCommand(ImageInfo *image_info,
  int argc,char **argv,char **metadata,ExceptionInfo *exception)
{
#define NotInitialized  (unsigned int) (~0)
#define DestroyComposite() \
{ \
  RelinquishCompositeOptions(&composite_options); \
  DestroyImageStack(); \
  for (i=0; i < (long) argc; i++) \
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
    "InvalidArgument","`%s': %s",option,argument); \
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
    pend;

  MagickStatusType
    status;

  long
    j,
    k;

  register long
    i;

  /*
    Set default.
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
          (void) fprintf(stdout,"Copyright: %s\n\n",GetMagickCopyright());
          return(MagickFalse);
        }
    }
  if (argc < 4)
    {
      (void) CompositeUsage();
      return(MagickTrue);
    }
  GetCompositeOptions(&composite_options);
  filename=(char *) NULL;
  format="%w,%h,%m";
  j=1;
  k=0;
  NewImageStack();
  option=(char *) NULL;
  pend=MagickFalse;
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
  for (i=1; i < (long) (argc-1); i++)
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
    if (IsMagickOption(option) == MagickFalse)
      {
        Image
          *images;

        /*
          Read input image.
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
            if (i == (long) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
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
              ThrowCompositeException(OptionError,"MissingArgument",option);
            type=ParseMagickOption(MagickAlphaOptions,MagickFalse,argv[i]);
            if (type < 0)
              ThrowCompositeException(OptionError,
                "UnrecognizedAlphaChannelType",argv[i]);
            break;
          }
        if (LocaleCompare("authenticate",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
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
            if (i == (long) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("blend",option+1) == 0)
          {
            (void) CloneString(&composite_options.compose_args,(char *) NULL);
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
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
            if (i == (long) argc)
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
            if (i == (long) argc)
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
            if (i == (long) (argc-1))
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
            if (i == (long) (argc-1))
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
            if (i == (long) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("channel",option+1) == 0)
          {
            long
              channel;

            if (*option == '+')
              {
                composite_options.channel=DefaultChannels;
                break;
              }
            i++;
            if (i == (long) (argc-1))
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
            if (i == (long) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
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
              ThrowCompositeException(OptionError,"MissingArgument",option);
            colorspace=ParseMagickOption(MagickColorspaceOptions,
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
            if (i == (long) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("compose",option+1) == 0)
          {
            long
              compose;

            composite_options.compose=UndefinedCompositeOp;
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            compose=ParseMagickOption(MagickComposeOptions,MagickFalse,
              argv[i]);
            if (compose < 0)
              ThrowCompositeException(OptionError,"UnrecognizedComposeOperator",
                argv[i]);
            composite_options.compose=(CompositeOperator) compose;
            break;
          }
        if (LocaleCompare("compress",option+1) == 0)
          {
            long
              compress;

            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            compress=ParseMagickOption(MagickCompressOptions,MagickFalse,
              argv[i]);
            if (compress < 0)
              ThrowCompositeException(OptionError,
                "UnrecognizedImageCompression",argv[i]);
            break;
          }
        ThrowCompositeException(OptionError,"UnrecognizedOption",option)
      }
      case 'd':
      {
        if (LocaleCompare("debug",option+1) == 0)
          {
            long
              event;

            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            event=ParseMagickOption(MagickLogEventOptions,MagickFalse,argv[i]);
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
            if (i == (long) (argc-1))
              ThrowCompositeException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("define",option+1) == 0)
          {
            i++;
            if (i == (long) argc)
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
            if (i == (long) argc)
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
            if (i == (long) argc)
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
            if (i == (long) argc)
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
            if (i == (long) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
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
              ThrowCompositeException(OptionError,"MissingArgument",option);
            dispose=ParseMagickOption(MagickDisposeOptions,MagickFalse,argv[i]);
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
            if (i == (long) argc)
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
            if (i == (long) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            (void) CloneString(&composite_options.compose_args,argv[i]);
            composite_options.compose=DistortCompositeOp;
            break;
          }
        if (LocaleCompare("method",option+1) == 0)
          {
            long
              method;

            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            method=ParseMagickOption(MagickDitherOptions,MagickFalse,argv[i]);
            if (method < 0)
              ThrowCompositeException(OptionError,"UnrecognizedDitherMethod",
                argv[i]);
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
            if (i == (long) (argc-1))
              ThrowCompositeException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("encoding",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
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
              ThrowCompositeException(OptionError,"MissingArgument",option);
            endian=ParseMagickOption(MagickEndianOptions,MagickFalse,
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
            if (i == (long) argc)
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
            long
              filter;

            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            filter=ParseMagickOption(MagickFilterOptions,MagickFalse,argv[i]);
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
            if (i == (long) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("format",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
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
            if (i == (long) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            (void) CloneString(&composite_options.geometry,argv[i]);
            break;
          }
        if (LocaleCompare("gravity",option+1) == 0)
          {
            long
              gravity;

            composite_options.gravity=UndefinedGravity;
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            gravity=ParseMagickOption(MagickGravityOptions,MagickFalse,
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
            if (i == (long) argc)
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
            long
              interlace;

            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            interlace=ParseMagickOption(MagickInterlaceOptions,MagickFalse,
              argv[i]);
            if (interlace < 0)
              ThrowCompositeException(OptionError,
                "UnrecognizedInterlaceType",argv[i]);
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
              ThrowCompositeException(OptionError,"MissingArgument",option);
            interpolate=ParseMagickOption(MagickInterpolateOptions,MagickFalse,
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
            if (i == (long) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
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
              ThrowCompositeException(OptionError,"MissingArgument",option);
            resource=ParseMagickOption(MagickResourceOptions,MagickFalse,
              argv[i]);
            if (resource < 0)
              ThrowCompositeException(OptionError,"UnrecognizedResourceType",
                argv[i]);
            i++;
            if (i == (long) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            value=strtod(argv[i],&p);
            if ((p == argv[i]) && (LocaleCompare("unlimited",argv[i]) != 0))
              ThrowCompositeInvalidArgumentException(option,argv[i]);
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
              ThrowCompositeException(OptionError,"MissingArgument",option);
            list=ParseMagickOption(MagickListOptions,MagickFalse,argv[i]);
            if (list < 0)
              ThrowCompositeException(OptionError,"UnrecognizedListType",
                argv[i]);
            (void) MogrifyImageInfo(image_info,(int) (i-j+1),(const char **)
              argv+j,exception);
            DestroyComposite();
            return(MagickTrue);
          }
        if (LocaleCompare("log",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if ((i == (long) argc) || (strchr(argv[i],'%') == (char *) NULL))
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
            if (i == (long) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("pointsize",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) (argc-1))
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
            if (i == (long) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("profile",option+1) == 0)
          {
            i++;
            if (i == (long) argc)
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
            if (i == (long) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
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
              ThrowCompositeException(OptionError,"MissingArgument",option);
            colorspace=ParseMagickOption(MagickColorspaceOptions,
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
            if (i == (long) argc)
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
            if (i == (long) argc)
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
            if (i == (long) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("rotate",option+1) == 0)
          {
            i++;
            if (i == (long) argc)
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
            if (i == (long) argc)
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
            if (i == (long) argc)
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
            if (i == (long) (argc-1))
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("sharpen",option+1) == 0)
          {
            i++;
            if (i == (long) argc)
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
            if (i == (long) (argc-1))
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
            if (i == (long) argc)
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
            if (i == (long) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            composite_options.stegano=atol(argv[i])+1;
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
            if (i == (long) argc)
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
            if (i == (long) (argc-1))
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowCompositeException(OptionError,"UnrecognizedOption",option)
      }
      case 't':
      {
        if (LocaleCompare("thumbnail",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("tile",option+1) == 0)
          {
            composite_options.tile=(*option == '-') ? MagickTrue : MagickFalse;
            (void) CopyMagickString(argv[i]+1,"sans",MaxTextExtent);
            break;
          }
        if (LocaleCompare("transform",option+1) == 0)
          break;
        if (LocaleCompare("transparent-color",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) (argc-1))
              ThrowCompositeException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("treedepth",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("type",option+1) == 0)
          {
            long
              type;

            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            type=ParseMagickOption(MagickTypeOptions,MagickFalse,argv[i]);
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
            long
              units;

            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            units=ParseMagickOption(MagickResolutionOptions,MagickFalse,
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
            if (i == (long) argc)
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
            (void) fprintf(stdout,"Version: %s\n",
              GetMagickVersion((unsigned long *) NULL));
            (void) fprintf(stdout,"Copyright: %s\n\n",GetMagickCopyright());
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
              ThrowCompositeException(OptionError,"MissingArgument",option);
            method=ParseMagickOption(MagickVirtualPixelOptions,MagickFalse,
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
            if (i == (long) argc)
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
            if (i == (long) argc)
              ThrowCompositeException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowCompositeInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("write",option+1) == 0)
          {
            i++;
            if (i == (long) argc)
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
    fire=ParseMagickOption(MagickImageListOptions,MagickFalse,option+1) < 0 ?
      MagickFalse : MagickTrue;
    if (fire != MagickFalse)
      FireImageStack(MagickFalse,MagickTrue,MagickTrue);
  }
  if (k != 0)
    ThrowCompositeException(OptionError,"UnbalancedParenthesis",argv[i]);
  if (i-- != (long) (argc-1))
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
  (void) TransformImage(&composite_image,(char *) NULL,
    composite_image->geometry);
  RemoveImageStack(mask_image);
  if (mask_image != (Image *) NULL)
    {
      if ((composite_options.compose == DisplaceCompositeOp) ||
          (composite_options.compose == DistortCompositeOp))
        {
          /*
            Merge Y displacement into X displacement image.
          */
          (void) CompositeImage(composite_image,CopyGreenCompositeOp,mask_image,
            0,0);
          mask_image=DestroyImage(mask_image);
        }
      else
        {
          /*
            Set a blending mask for the composition.
          */
          images->mask=mask_image;
          (void) NegateImage(images->mask,MagickFalse);
        }
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

      text=InterpretImageProperties(image_info,images,format);
      if (text == (char *) NULL)
        ThrowCompositeException(ResourceLimitError,"MemoryAllocationFailed",
          GetExceptionMessage(errno));
      (void) ConcatenateString(&(*metadata),text);
      (void) ConcatenateString(&(*metadata),"\n");
      text=DestroyString(text);
    }
  images=DestroyImage(images);
  RelinquishCompositeOptions(&composite_options);
  DestroyComposite();
  return(status != 0 ? MagickTrue : MagickFalse);
}
