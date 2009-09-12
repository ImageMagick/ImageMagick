/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%               M   M   OOO   N   N  TTTTT   AAA    GGGG  EEEEE               %
%               MM MM  O   O  NN  N    T    A   A  G      E                   %
%               M M M  O   O  N N N    T    AAAAA  G  GG  EEE                 %
%               M   M  O   O  N  NN    T    A   A  G   G  E                   %
%               M   M   OOO   N   N    T    A   A   GGG   EEEEE               %
%                                                                             %
%                                                                             %
%                MagickWand Methods to Create Image Thumbnails                %
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
%  Use the montage program to create a composite image by combining several
%  separate images. The images are tiled on the composite image optionally
%  adorned with a border, frame, image name, and more.
%
*/

/*
  Include declarations.
*/
#include "wand/studio.h"
#include "wand/MagickWand.h"
#include "wand/mogrify-private.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+    M o n t a g e I m a g e C o m m a n d                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MontageImageCommand() reads one or more images, applies one or more image
%  processing operations, and writes out the image in the same or
%  differing format.
%
%  The format of the MontageImageCommand method is:
%
%      MagickBooleanType MontageImageCommand(ImageInfo *image_info,int argc,
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

static MagickBooleanType MontageUsage(void)
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
      "-adaptive-sharpen geometry",
      "                     adaptively sharpen pixels; increase effect near edges",
      " annotate geometry text",
      "                     annotate the image with text",
      "-auto-orient         automagically orient image",
      "-blur geometry      reduce image noise and reduce detail levels",
      "-border geometry     surround image with a border of color",
      "-crop geometry       preferred size and location of the cropped image",
      "-extent geometry     set the image size",
      "-flatten             flatten a sequence of images",
      "-flip                flip image in the vertical direction",
      "-flop                flop image in the horizontal direction",
      "-frame geometry      surround image with an ornamental border",
      "-monochrome          transform image to black and white",
      "-polaroid angle      simulate a Polaroid picture",
      "-repage geometry     size and location of an image canvas (operator)",
      "-resize geometry     resize the image",
      "-rotate degrees      apply Paeth rotation to the image",
      "-strip               strip image of all profiles and comments",
      "-transform           affine transform image",
      "-transpose           flip image vertically and rotate 90 degrees",
      "-transparent color   make this color transparent within the image",
      "-type type           image type",
      "-unsharp geometry    sharpen the image",
      (char *) NULL
    },
    *settings[]=
    {
      "-adjoin              join images into a single multi-image file",
      "-affine matrix       affine transform matrix",
      "-alpha option        on, activate, off, deactivate, set, opaque, copy",
      "                     transparent, extract, background, or shape",
      "-authenticate password",
      "                     decipher image with this password",
      "-blue-primary point  chromaticity blue primary point",
      "-bordercolor color   border color",
      "-caption string      assign a caption to an image",
      "-channel type        apply option to select image channels",
      "-colors value        preferred number of colors in the image",
      "-colorspace type     alternate image colorsapce",
      "-comment string      annotate image with comment",
      "-compose operator    composite operator",
      "-compress type       type of pixel compression when writing the image",
      "-define format:option",
      "                     define one or more image format options",
      "-density geometry    horizontal and vertical density of the image",
      "-depth value         image depth",
      "-display server      query font from this X server",
      "-dispose method      layer disposal method",
      "-dither method       apply error diffusion to image",
      "-draw string         annotate the image with a graphic primitive",
      "-encoding type       text encoding type",
      "-endian type         endianness (MSB or LSB) of the image",
      "-extract geometry    extract area from image",
      "-fill color          color to use when filling a graphic primitive",
      "-filter type         use this filter when resizing an image",
      "-font name           render text with this font",
      "-format \"string\"     output formatted image characteristics",
      "-gamma value         level of gamma correction",
      "-geometry geometry   preferred tile and border sizes",
      "-gravity direction   which direction to gravitate towards",
      "-green-primary point chromaticity green primary point",
      "-identify            identify the format and characteristics of the image",
      "-interlace type      type of image interlacing scheme",
      "-interpolate method  pixel color interpolation method",
      "-label string        assign a label to an image",
      "-limit type value    pixel cache resource limit",
      "-mattecolor color    frame color",
      "-mode type           framing style",
      "-monitor             monitor progress",
      "-origin geometry     image origin",
      "-page geometry       size and location of an image canvas (setting)",
      "-pointsize value     font point size",
      "-profile filename    add, delete, or apply an image profile",
      "-quality value       JPEG/MIFF/PNG compression level",
      "-quantize colorspace reduce colors in this colorspace",
      "-quiet               suppress all warning messages",
      "-red-primary point   chromaticity red primary point",
      "-regard-warnings     pay attention to warning messages",
      "-respect-parentheses settings remain in effect until parenthesis boundary",
      "-sampling-factor geometry",
      "                     horizontal and vertical sampling factor",
      "-scenes range        image scene range",
      "-seed value          seed a new sequence of pseudo-random numbers",
      "-set attribute value set an image attribute",
      "-shadow              add a shadow beneath a tile to simulate depth",
      "-size geometry       width and height of image",
      "-stroke color        color to use when stroking a graphic primitive",
      "-texture filename    name of texture to tile onto the image background",
      "-thumbnail geometry  create a thumbnail of the image",
      "-tile geometry       number of tiles per row and column",
      "-title string        decorate the montage image with a title",
      "-transparent-color color",
      "                     transparent color",
      "-treedepth value     color tree depth",
      "-trim                trim image edges",
      "-units type          the units of image resolution",
      "-verbose             print detailed information about the image",
      "-virtual-pixel method",
      "                     virtual pixel access method",
      "-white-point point   chromaticity white point",
      (char *) NULL
    },
    *sequence_operators[]=
    {
      "-coalesce            merge a sequence of images",
      "-composite           composite image",
      (char *) NULL
    },
    *stack_operators[]=
    {
      "-clone index         clone an image",
      (char *) NULL
    };

  (void) printf("Version: %s\n",GetMagickVersion((unsigned long *) NULL));
  (void) printf("Copyright: %s\n\n",GetMagickCopyright());
  (void) printf("Usage: %s [options ...] file [ [options ...] file ...] file\n",
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
    "\nIn addition to those listed above, you can specify these standard X\n");
  (void) printf(
    "resources as command line options:  -background, -bordercolor,\n");
  (void) printf(
    "-borderwidth, -font, -mattecolor, or -title\n");
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

WandExport MagickBooleanType MontageImageCommand(ImageInfo *image_info,
  int argc,char **argv,char **metadata,ExceptionInfo *exception)
{
#define DestroyMontage() \
{ \
  if (montage_image != (Image *) NULL) \
    montage_image=DestroyImageList(montage_image); \
  DestroyImageStack(); \
  for (i=0; i < (long) argc; i++) \
    argv[i]=DestroyString(argv[i]); \
  argv=(char **) RelinquishMagickMemory(argv); \
}
#define ThrowMontageException(asperity,tag,option) \
{ \
  (void) ThrowMagickException(exception,GetMagickModule(),asperity,tag,"`%s'", \
    option); \
  DestroyMontage(); \
  return(MagickFalse); \
}
#define ThrowMontageInvalidArgumentException(option,argument) \
{ \
  (void) ThrowMagickException(exception,GetMagickModule(),OptionError, \
    "InvalidArgument","`%s': %s",option,argument); \
  DestroyMontage(); \
  return(MagickFalse); \
}

  char
    *option,
    *transparent_color;

  const char
    *format;

  Image
    *image,
    *montage_image;

  ImageStack
    image_stack[MaxImageStackDepth+1];

  long
    first_scene,
    j,
    k,
    last_scene,
    scene;

  MagickBooleanType
    fire,
    pend;

  MagickStatusType
    status;

  MontageInfo
    *montage_info;

  register long
    i;

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
          (void) fprintf(stdout,"Copyright: %s\n\n",GetMagickCopyright());
          return(MagickFalse);
        }
    }
  if (argc < 3)
    {
      (void) MontageUsage();
      return(MagickTrue);
    }
  format="%w,%h,%m";
  first_scene=0;
  j=1;
  k=0;
  last_scene=0;
  montage_image=NewImageList();
  montage_info=CloneMontageInfo(image_info,(MontageInfo *) NULL);
  NewImageStack();
  option=(char *) NULL;
  pend=MagickFalse;
  scene=0;
  status=MagickFalse;
  transparent_color=(char *) NULL;
  /*
    Parse command line.
  */
  ReadCommandlLine(argc,&argv);
  status=ExpandFilenames(&argc,&argv);
  if (status == MagickFalse)
    ThrowMontageException(ResourceLimitError,"MemoryAllocationFailed",
      GetExceptionMessage(errno));
  for (i=1; i < (long) (argc-1); i++)
  {
    option=argv[i];
    if (LocaleCompare(option,"(") == 0)
      {
        FireImageStack(MagickTrue,MagickTrue,pend);
        if (k == MaxImageStackDepth)
          ThrowMontageException(OptionError,"ParenthesisNestedTooDeeply",
            option);
        PushImageStack();
        continue;
      }
    if (LocaleCompare(option,")") == 0)
      {
        FireImageStack(MagickTrue,MagickTrue,MagickTrue);
        if (k == 0)
          ThrowMontageException(OptionError,"UnableToParseExpression",option);
        PopImageStack();
        continue;
      }
    if (IsMagickOption(option) == MagickFalse)
      {
        Image
          *images;

        FireImageStack(MagickFalse,MagickFalse,pend);
        for (scene=first_scene; scene <= last_scene ; scene++)
        {
          char
            *filename;

          /*
            Option is a file name: begin by reading image from specified file.
          */
          filename=argv[i];
          if ((LocaleCompare(filename,"--") == 0) && (i < (argc-1)))
            filename=argv[++i];
          (void) CopyMagickString(image_info->filename,filename,MaxTextExtent);
          if (first_scene != last_scene)
            {
              char
                filename[MaxTextExtent];

              /*
                Form filename for multi-part images.
              */
              (void) InterpretImageFilename(image_info,(Image *) NULL,
                image_info->filename,(int) scene,filename);
              if (LocaleCompare(filename,image_info->filename) == 0)
                (void) FormatMagickString(filename,MaxTextExtent,"%s.%lu",
                  image_info->filename,scene);
              (void) CopyMagickString(image_info->filename,filename,
                MaxTextExtent);
            }
          (void) CloneString(&image_info->font,montage_info->font);
          images=ReadImages(image_info,exception);
          status&=(images != (Image *) NULL) &&
            (exception->severity < ErrorException);
          if (images == (Image *) NULL)
            continue;
          AppendImageStack(images);
        }
        continue;
      }
    pend=image != (Image *) NULL ? MagickTrue : MagickFalse;
    switch (*(option+1))
    {
      case 'a':
      {
        if (LocaleCompare("adaptive-sharpen",option+1) == 0)
          {
            i++;
            if (i == (long) (argc-1))
              ThrowMontageException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMontageInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("adjoin",option+1) == 0)
          break;
        if (LocaleCompare("affine",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) (argc-1))
              ThrowMontageException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMontageInvalidArgumentException(option,argv[i]);
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
              ThrowMontageException(OptionError,"MissingArgument",option);
            type=ParseMagickOption(MagickAlphaOptions,MagickFalse,argv[i]);
            if (type < 0)
              ThrowMontageException(OptionError,"UnrecognizedAlphaChannelType",
                argv[i]);
            break;
          }
        if (LocaleCompare("annotate",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) (argc-1))
              ThrowMontageException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMontageInvalidArgumentException(option,argv[i]);
            if (i == (long) (argc-1))
              ThrowMontageException(OptionError,"MissingArgument",option);
            i++;
            break;
          }
        if (LocaleCompare("auto-orient",option+1) == 0)
          break;
        if (LocaleCompare("authenticate",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            break;
          }
        ThrowMontageException(OptionError,"UnrecognizedOption",option)
      }
      case 'b':
      {
        if (LocaleCompare("background",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            (void) QueryColorDatabase(argv[i],
              &montage_info->background_color,exception);
            break;
          }
        if (LocaleCompare("blue-primary",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMontageInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("blur",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMontageInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("border",option+1) == 0)
          {
            (void) CopyMagickString(argv[i]+1,"sans",MaxTextExtent);
            montage_info->border_width=0;
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMontageInvalidArgumentException(option,argv[i]);
            montage_info->border_width=(unsigned long) atol(argv[i]);
            break;
          }
        if (LocaleCompare("bordercolor",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            (void) QueryColorDatabase(argv[i],&montage_info->border_color,
              exception);
            break;
          }
        if (LocaleCompare("borderwidth",option+1) == 0)
          {
            montage_info->border_width=0;
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMontageInvalidArgumentException(option,argv[i]);
            montage_info->border_width=(unsigned long) atol(argv[i]);
            break;
          }
        ThrowMontageException(OptionError,"UnrecognizedOption",option)
      }
      case 'c':
      {
        if (LocaleCompare("cache",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMontageInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("caption",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
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
              ThrowMontageException(OptionError,"MissingArgument",option);
            channel=ParseChannelOption(argv[i]);
            if (channel < 0)
              ThrowMontageException(OptionError,"UnrecognizedChannelType",
                argv[i]);
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
              ThrowMontageException(ImageError,"ImageSequenceRequired",option);
            FireImageStack(MagickTrue,MagickTrue,MagickTrue);
            if (*option == '+')
              clone_images=CloneImages(clone_images,"-1",exception);
            else
              {
                i++;
                if (i == (long) (argc-1))
                  ThrowMontageException(OptionError,"MissingArgument",option);
                if (IsSceneGeometry(argv[i],MagickFalse) == MagickFalse)
                  ThrowMontageInvalidArgumentException(option,argv[i]);
                clone_images=CloneImages(clone_images,argv[i],exception);
              }
            if (clone_images == (Image *) NULL)
              ThrowMontageException(OptionError,"NoSuchImage",option);
            AppendImageStack(clone_images);
            break;
          }
        if (LocaleCompare("coalesce",option+1) == 0)
          break;
        if (LocaleCompare("colors",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMontageInvalidArgumentException(option,argv[i]);
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
              ThrowMontageException(OptionError,"MissingArgument",option);
            colorspace=ParseMagickOption(MagickColorspaceOptions,
              MagickFalse,argv[i]);
            if (colorspace < 0)
              ThrowMontageException(OptionError,"UnrecognizedColorspace",
                argv[i]);
            break;
          }
        if (LocaleCompare("comment",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("compose",option+1) == 0)
          {
            long
              compose;

            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            compose=ParseMagickOption(MagickComposeOptions,MagickFalse,argv[i]);
            if (compose < 0)
              ThrowMontageException(OptionError,"UnrecognizedComposeOperator",
                argv[i]);
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
              ThrowMontageException(OptionError,"MissingArgument",option);
            compress=ParseMagickOption(MagickCompressOptions,MagickFalse,
              argv[i]);
            if (compress < 0)
              ThrowMontageException(OptionError,"UnrecognizedCompressType",
                argv[i]);
            break;
          }
        if (LocaleCompare("crop",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMontageInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowMontageException(OptionError,"UnrecognizedOption",option)
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
              ThrowMontageException(OptionError,"MissingArgument",option);
            event=ParseMagickOption(MagickLogEventOptions,MagickFalse,argv[i]);
            if (event < 0)
              ThrowMontageException(OptionError,"UnrecognizedEventType",
                argv[i]);
            (void) SetLogEventMask(argv[i]);
            break;
          }
        if (LocaleCompare("define",option+1) == 0)
          {
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            if (*option == '+')
              {
                const char
                  *define;

                define=GetImageOption(image_info,argv[i]);
                if (define == (const char *) NULL)
                  ThrowMontageException(OptionError,"NoSuchOption",argv[i]);
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
              ThrowMontageException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMontageInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("depth",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMontageInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("display",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
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
              ThrowMontageException(OptionError,"MissingArgument",option);
            dispose=ParseMagickOption(MagickDisposeOptions,MagickFalse,argv[i]);
            if (dispose < 0)
              ThrowMontageException(OptionError,"UnrecognizedDisposeMethod",
                argv[i]);
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
              ThrowMontageException(OptionError,"MissingArgument",option);
            method=ParseMagickOption(MagickDitherOptions,MagickFalse,argv[i]);
            if (method < 0)
              ThrowMontageException(OptionError,"UnrecognizedDitherMethod",
                argv[i]);
            break;
          }
        if (LocaleCompare("draw",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            break;
          }
        ThrowMontageException(OptionError,"UnrecognizedOption",option)
      }
      case 'e':
      {
        if (LocaleCompare("encoding",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
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
              ThrowMontageException(OptionError,"MissingArgument",option);
            endian=ParseMagickOption(MagickEndianOptions,MagickFalse,
              argv[i]);
            if (endian < 0)
              ThrowMontageException(OptionError,"UnrecognizedEndianType",
                argv[i]);
            break;
          }
        if (LocaleCompare("extent",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) (argc-1))
              ThrowMontageException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMontageInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowMontageException(OptionError,"UnrecognizedOption",option)
      }
      case 'f':
      {
        if (LocaleCompare("fill",option+1) == 0)
          {
            (void) QueryColorDatabase("none",&montage_info->fill,exception);
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            (void) QueryColorDatabase(argv[i],&montage_info->fill,
              exception);
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
              ThrowMontageException(OptionError,"MissingArgument",option);
            filter=ParseMagickOption(MagickFilterOptions,MagickFalse,argv[i]);
            if (filter < 0)
              ThrowMontageException(OptionError,"UnrecognizedImageFilter",
                argv[i]);
            break;
          }
        if (LocaleCompare("flatten",option+1) == 0)
          break;
        if (LocaleCompare("flip",option+1) == 0)
          break;
        if (LocaleCompare("flop",option+1) == 0)
          break;
        if (LocaleCompare("font",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            (void) CloneString(&montage_info->font,argv[i]);
            break;
          }
        if (LocaleCompare("format",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            format=argv[i];
            break;
          }
        if (LocaleCompare("frame",option+1) == 0)
          {
            (void) CopyMagickString(argv[i]+1,"sans",MaxTextExtent);
            (void) CloneString(&montage_info->frame,(char *) NULL);
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMontageInvalidArgumentException(option,argv[i]);
            (void) CloneString(&montage_info->frame,argv[i]);
            break;
          }
        ThrowMontageException(OptionError,"UnrecognizedOption",option)
      }
      case 'g':
      {
        if (LocaleCompare("gamma",option+1) == 0)
          {
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMontageInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("geometry",option+1) == 0)
          {
            (void) CloneString(&montage_info->geometry,(char *) NULL);
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMontageInvalidArgumentException(option,argv[i]);
            (void) CloneString(&montage_info->geometry,argv[i]);
            break;
          }
        if (LocaleCompare("gravity",option+1) == 0)
          {
            long
              gravity;

            montage_info->gravity=UndefinedGravity;
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            gravity=ParseMagickOption(MagickGravityOptions,MagickFalse,
              argv[i]);
            if (gravity < 0)
              ThrowMontageException(OptionError,"UnrecognizedGravityType",
                argv[i]);
            montage_info->gravity=(GravityType) gravity;
            break;
          }
        if (LocaleCompare("green-primary",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMontageInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowMontageException(OptionError,"UnrecognizedOption",option)
      }
      case 'h':
      {
        if ((LocaleCompare("help",option+1) == 0) ||
            (LocaleCompare("-help",option+1) == 0))
          return(MontageUsage());
        ThrowMontageException(OptionError,"UnrecognizedOption",option)
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
              ThrowMontageException(OptionError,"MissingArgument",option);
            interlace=ParseMagickOption(MagickInterlaceOptions,MagickFalse,
              argv[i]);
            if (interlace < 0)
              ThrowMontageException(OptionError,"UnrecognizedInterlaceType",
                argv[i]);
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
              ThrowMontageException(OptionError,"MissingArgument",option);
            interpolate=ParseMagickOption(MagickInterpolateOptions,MagickFalse,
              argv[i]);
            if (interpolate < 0)
              ThrowMontageException(OptionError,"UnrecognizedInterpolateMethod",
                argv[i]);
            break;
          }
        ThrowMontageException(OptionError,"UnrecognizedOption",option)
      }
      case 'l':
      {
        if (LocaleCompare("label",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
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
              ThrowMontageException(OptionError,"MissingArgument",option);
            resource=ParseMagickOption(MagickResourceOptions,MagickFalse,
              argv[i]);
            if (resource < 0)
              ThrowMontageException(OptionError,"UnrecognizedResourceType",
                argv[i]);
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            value=strtod(argv[i],&p);
            if ((p == argv[i]) && (LocaleCompare("unlimited",argv[i]) != 0))
              ThrowMontageInvalidArgumentException(option,argv[i]);
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
              ThrowMontageException(OptionError,"MissingArgument",option);
            list=ParseMagickOption(MagickListOptions,MagickFalse,argv[i]);
            if (list < 0)
              ThrowMontageException(OptionError,"UnrecognizedListType",argv[i]);
            (void) MogrifyImageInfo(image_info,(int) (i-j+1),(const char **)
              argv+j,exception);
            DestroyMontage();
            return(MagickTrue);
          }
        if (LocaleCompare("log",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if ((i == (long) argc) ||
                (strchr(argv[i],'%') == (char *) NULL))
              ThrowMontageException(OptionError,"MissingArgument",option);
            break;
          }
        ThrowMontageException(OptionError,"UnrecognizedOption",option)
      }
      case 'm':
      {
        if (LocaleCompare("matte",option+1) == 0)
          break;
        if (LocaleCompare("mattecolor",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            (void) QueryColorDatabase(argv[i],&montage_info->matte_color,
              exception);
            break;
          }
        if (LocaleCompare("mode",option+1) == 0)
          {
            MontageMode
              mode;

            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            mode=UndefinedMode;
            if (LocaleCompare("frame",argv[i]) == 0)
              {
                mode=FrameMode;
                (void) CloneString(&montage_info->frame,"15x15+3+3");
                montage_info->shadow=MagickTrue;
                break;
              }
            if (LocaleCompare("unframe",argv[i]) == 0)
              {
                mode=UnframeMode;
                montage_info->frame=(char *) NULL;
                montage_info->shadow=MagickFalse;
                montage_info->border_width=0;
                break;
              }
            if (LocaleCompare("concatenate",argv[i]) == 0)
              {
                mode=ConcatenateMode;
                montage_info->frame=(char *) NULL;
                montage_info->shadow=MagickFalse;
                montage_info->gravity=(GravityType) NorthWestGravity;
                (void) CloneString(&montage_info->geometry,"+0+0");
                montage_info->border_width=0;
                break;
              }
            if (mode == UndefinedMode)
              ThrowMontageException(OptionError,"UnrecognizedImageMode",
                argv[i]);
            break;
          }
        if (LocaleCompare("monitor",option+1) == 0)
          break;
        if (LocaleCompare("monochrome",option+1) == 0)
          {
            if (*option == '+')
              break;
            break;
          }
        ThrowMontageException(OptionError,"UnrecognizedOption",option)
      }
      case 'n':
      {
        if (LocaleCompare("noop",option+1) == 0)
          break;
        ThrowMontageException(OptionError,"UnrecognizedOption",option)
      }
      case 'o':
      {
        if (LocaleCompare("origin",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMontageInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowMontageException(OptionError,"UnrecognizedOption",option)
      }
      case 'p':
      {
        if (LocaleCompare("page",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("pointsize",option+1) == 0)
          {
            montage_info->pointsize=12;
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMontageInvalidArgumentException(option,argv[i]);
            montage_info->pointsize=atof(argv[i]);
            break;
          }
        if (LocaleCompare("polaroid",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) (argc-1))
              ThrowMontageException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMontageInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("profile",option+1) == 0)
          {
            i++;
            if (i == (long) (argc-1))
              ThrowMontageException(OptionError,"MissingArgument",option);
            break;
          }
        ThrowMontageException(OptionError,"UnrecognizedOption",option)
      }
      case 'q':
      {
        if (LocaleCompare("quality",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMontageInvalidArgumentException(option,argv[i]);
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
              ThrowMontageException(OptionError,"MissingArgument",option);
            colorspace=ParseMagickOption(MagickColorspaceOptions,
              MagickFalse,argv[i]);
            if (colorspace < 0)
              ThrowMontageException(OptionError,"UnrecognizedColorspace",
                argv[i]);
            break;
          }
        if (LocaleCompare("quiet",option+1) == 0)
          break;
        ThrowMontageException(OptionError,"UnrecognizedOption",option)
      }
      case 'r':
      {
        if (LocaleCompare("red-primary",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMontageInvalidArgumentException(option,argv[i]);
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
              ThrowMontageException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMontageInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("resize",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMontageInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleNCompare("respect-parentheses",option+1,17) == 0)
          {
            respect_parenthesis=(*option == '-') ? MagickTrue : MagickFalse;
            break;
          }
        if (LocaleCompare("rotate",option+1) == 0)
          {
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMontageInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowMontageException(OptionError,"UnrecognizedOption",option)
      }
      case 's':
      {
        if (LocaleCompare("sampling-factor",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMontageInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("seed",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) (argc-1))
              ThrowMontageException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMontageInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("scenes",option+1) == 0)
          {
            first_scene=0;
            last_scene=0;
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            if (IsSceneGeometry(argv[i],MagickFalse) == MagickFalse)
              ThrowMontageInvalidArgumentException(option,argv[i]);
            first_scene=atol(argv[i]);
            last_scene=first_scene;
            (void) sscanf(argv[i],"%ld-%ld",&first_scene,&last_scene);
            break;
          }
        if (LocaleCompare("set",option+1) == 0)
          {
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("shadow",option+1) == 0)
          {
            (void) CopyMagickString(argv[i]+1,"sans",MaxTextExtent);
            montage_info->shadow=(*option == '-') ? MagickTrue : MagickFalse;
            break;
          }
        if (LocaleCompare("sharpen",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if ((i == (long) argc) || (IsGeometry(argv[i]) == MagickFalse))
              ThrowMontageException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("size",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMontageInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("stroke",option+1) == 0)
          {
            (void) QueryColorDatabase("none",&montage_info->stroke,exception);
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            (void) QueryColorDatabase(argv[i],&montage_info->stroke,
              exception);
            break;
          }
        if (LocaleCompare("strip",option+1) == 0)
          break;
        if (LocaleCompare("strokewidth",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMontageInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("support",option+1) == 0)
          {
            i++;  /* deprecated */
            break;
          }
        ThrowMontageException(OptionError,"UnrecognizedOption",option)
      }
      case 't':
      {
        if (LocaleCompare("texture",option+1) == 0)
          {
            (void) CloneString(&montage_info->texture,(char *) NULL);
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            (void) CloneString(&montage_info->texture,argv[i]);
            break;
          }
        if (LocaleCompare("thumbnail",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMontageInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("tile",option+1) == 0)
          {
            (void) CopyMagickString(argv[i]+1,"sans",MaxTextExtent);
            (void) CloneString(&montage_info->tile,(char *) NULL);
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMontageInvalidArgumentException(option,argv[i]);
            (void) CloneString(&montage_info->tile,argv[i]);
            break;
          }
        if (LocaleCompare("tile-offset",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) (argc-1))
              ThrowMontageException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMontageInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("tint",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMontageInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("transform",option+1) == 0)
          break;
        if (LocaleCompare("transpose",option+1) == 0)
          break;
        if (LocaleCompare("title",option+1) == 0)
          {
            (void) CloneString(&montage_info->title,(char *) NULL);
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            (void) CloneString(&montage_info->title,argv[i]);
            break;
          }
        if (LocaleCompare("transform",option+1) == 0)
          break;
        if (LocaleCompare("transparent",option+1) == 0)
          {
            transparent_color=(char *) NULL;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            (void) CloneString(&transparent_color,argv[i]);
            break;
          }
        if (LocaleCompare("transparent-color",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) (argc-1))
              ThrowMontageException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("treedepth",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMontageInvalidArgumentException(option,argv[i]);
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
              ThrowMontageException(OptionError,"MissingArgument",option);
            type=ParseMagickOption(MagickTypeOptions,MagickFalse,argv[i]);
            if (type < 0)
              ThrowMontageException(OptionError,"UnrecognizedImageType",
                argv[i]);
            break;
          }
        ThrowMontageException(OptionError,"UnrecognizedOption",option)
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
            if (i == (long) (argc-1))
              ThrowMontageException(OptionError,"MissingArgument",option);
            units=ParseMagickOption(MagickResolutionOptions,MagickFalse,
              argv[i]);
            if (units < 0)
              ThrowMontageException(OptionError,"UnrecognizedUnitsType",
                argv[i]);
            break;
          }
        if (LocaleCompare("unsharp",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) (argc-1))
              ThrowMontageException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMontageInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowMontageException(OptionError,"UnrecognizedOption",option)
      }
      case 'v':
      {
        if (LocaleCompare("verbose",option+1) == 0)
          {
            break;
          }
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
              ThrowMontageException(OptionError,"MissingArgument",option);
            method=ParseMagickOption(MagickVirtualPixelOptions,MagickFalse,
              argv[i]);
            if (method < 0)
              ThrowMontageException(OptionError,
                "UnrecognizedVirtualPixelMethod",argv[i]);
            break;
          }
        ThrowMontageException(OptionError,"UnrecognizedOption",option)
      }
      case 'w':
      {
        if (LocaleCompare("white-point",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (long) argc)
              ThrowMontageException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowMontageInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowMontageException(OptionError,"UnrecognizedOption",option)
      }
      case '?':
        break;
      default:
        ThrowMontageException(OptionError,"UnrecognizedOption",option)
    }
    fire=ParseMagickOption(MagickImageListOptions,MagickFalse,option+1) < 0 ? 
      MagickFalse : MagickTrue;
    if (fire != MagickFalse)
      FireImageStack(MagickTrue,MagickTrue,MagickTrue);
  }
  if (k != 0)
    ThrowMontageException(OptionError,"UnbalancedParenthesis",argv[i]);
  if (i-- != (long) (argc-1))
    ThrowMontageException(OptionError,"MissingAnImageFilename",argv[i]);
  if (image == (Image *) NULL)
    ThrowMontageException(OptionError,"MissingAnImageFilename",argv[argc-1]);
  FinalizeImageSettings(image_info,image,MagickTrue);
  if (image == (Image *) NULL)
    ThrowMontageException(OptionError,"MissingAnImageFilename",argv[argc-1]);
  (void) CopyMagickString(montage_info->filename,argv[argc-1],MaxTextExtent);
  montage_image=MontageImageList(image_info,montage_info,image,exception);
  if (montage_image == (Image *) NULL)
    status=MagickFalse;
  else
    {
      /*
        Write image.
      */
      GetImageException(montage_image,exception);
      (void) CopyMagickString(image_info->filename,argv[argc-1],MaxTextExtent);
      (void) CopyMagickString(montage_image->magick_filename,argv[argc-1],
        MaxTextExtent);
      if (*montage_image->magick == '\0')
        (void) CopyMagickString(montage_image->magick,image->magick,
          MaxTextExtent);
      status&=WriteImages(image_info,montage_image,argv[argc-1],exception);
      if (metadata != (char **) NULL)
        {
          char
            *text;

          text=InterpretImageProperties(image_info,montage_image,format);
          if (text == (char *) NULL)
            ThrowMontageException(ResourceLimitError,"MemoryAllocationFailed",
              GetExceptionMessage(errno));
          (void) ConcatenateString(&(*metadata),text);
          (void) ConcatenateString(&(*metadata),"\n");
          text=DestroyString(text);
        }
    }
  montage_info=DestroyMontageInfo(montage_info);
  DestroyMontage();
  return(status != 0 ? MagickTrue : MagickFalse);
}
