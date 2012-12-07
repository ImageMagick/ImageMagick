/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                AAA   N   N  IIIII  M   M   AAA   TTTTT  EEEEE               %
%               A   A  NN  N    I    MM MM  A   A    T    E                   %
%               AAAAA  N N N    I    M M M  AAAAA    T    EEE                 %
%               A   A  N  NN    I    M   M  A   A    T    E                   %
%               A   A  N   N  IIIII  M   M  A   A    T    EEEEE               %
%                                                                             %
%                                                                             %
%              Methods to Interactively Animate an Image Sequence             %
%                                                                             %
%                             Software Design                                 %
%                               John Cristy                                   %
%                                July 1992                                    %
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
%  Use the animate program to animate an image sequence on any X server.
%
*/

/*
  Include declarations.
*/
#include "wand/studio.h"
#include "wand/MagickWand.h"
#include "wand/mogrify-private.h"
#include "magick/animate-private.h"
#include "magick/string-private.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   A n i m a t e I m a g e C o m m a n d                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  AnimateImageCommand() displays a sequence of images on any workstation
%  display running an X server. Animate first determines the hardware
%  capabilities of the workstation. If the number of unique colors in an image
%  is less than or equal to the number the workstation can support, the image
%  is displayed in an X window. Otherwise the number of colors in the image is
%  first reduced to match the color resolution of the workstation before it is
%  displayed.
%
%  This means that a continuous-tone 24 bits/pixel image can display on a 8
%  bit pseudo-color device or monochrome device. In most instances the reduced
%  color image closely resembles the original. Alternatively, a monochrome or
%  pseudo-color image sequence can display on a continuous-tone 24 bits/pixels
%  device.
%
%  The format of the AnimateImageCommand method is:
%
%      MagickBooleanType AnimateImageCommand(ImageInfo *image_info,int argc,
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

static MagickBooleanType AnimateUsage(void)
{
  const char
    **p;

  static const char
    *buttons[]=
    {
      "Press any button to map or unmap the Command widget",
      (char *) NULL
    },
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
      "-colors value        preferred number of colors in the image",
      "-crop geometry       preferred size and location of the cropped image",
      "-extract geometry    extract area from image",
      "-monochrome          transform image to black and white",
      "-repage geometry     size and location of an image canvas (operator)",
      "-resample geometry   change the resolution of an image",
      "-resize geometry     resize the image",
      "-rotate degrees      apply Paeth rotation to the image",
      "-strip               strip image of all profiles and comments",
      "-thumbnail geometry  create a thumbnail of the image",
      "-trim                trim image edges",
      (char *) NULL
    },
    *settings[]=
    {
      "-alpha option        on, activate, off, deactivate, set, opaque, copy",
      "                     transparent, extract, background, or shape",
      "-authenticate password",
      "                     decipher image with this password",
      "-backdrop            display image centered on a backdrop",
      "-channel type        apply option to select image channels",
      "-colormap type       Shared or Private",
      "-colorspace type     alternate image colorspace",
      "-decipher filename   convert cipher pixels to plain pixels",
      "-define format:option",
      "                     define one or more image format options",
      "-delay value         display the next image after pausing",
      "-density geometry    horizontal and vertical density of the image",
      "-depth value         image depth",
      "-display server      display image to this X server",
      "-dispose method      layer disposal method",
      "-dither method       apply error diffusion to image",
      "-filter type         use this filter when resizing an image",
      "-format \"string\"     output formatted image characteristics",
      "-gamma value         level of gamma correction",
      "-geometry geometry   preferred size and location of the Image window",
      "-gravity type        horizontal and vertical backdrop placement",
      "-identify            identify the format and characteristics of the image",
      "-immutable           displayed image cannot be modified",
      "-interlace type      type of image interlacing scheme",
      "-interpolate method  pixel color interpolation method",
      "-limit type value    pixel cache resource limit",
      "-loop iterations     loop images then exit",
      "-map type            display image using this Standard Colormap",
      "-monitor             monitor progress",
      "-pause               seconds to pause before reanimating",
      "-page geometry       size and location of an image canvas (setting)",
      "-quantize colorspace reduce colors in this colorspace",
      "-quiet               suppress all warning messages",
      "-regard-warnings     pay attention to warning messages",
      "-remote command      execute a command in an remote display process",
      "-respect-parentheses settings remain in effect until parenthesis boundary",
      "-sampling-factor geometry",
      "                     horizontal and vertical sampling factor",
      "-seed value          seed a new sequence of pseudo-random numbers",
      "-set attribute value set an image attribute",
      "-size geometry       width and height of image",
      "-transparent-color color",
      "                     transparent color",
      "-treedepth value     color tree depth",
      "-verbose             print detailed information about the image",
      "-visual type         display image using this visual type",
      "-virtual-pixel method",
      "                     virtual pixel access method",
      "-window id           display image to background of this window",
      (char *) NULL
    },
    *sequence_operators[]=
    {
      "-coalesce            merge a sequence of images",
      "-flatten             flatten a sequence of images",
      (char *) NULL
    };

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
  (void) printf("\nMiscellaneous Options:\n");
  for (p=miscellaneous; *p != (char *) NULL; p++)
    (void) printf("  %s\n",*p);
  (void) printf(
    "\nIn addition to those listed above, you can specify these standard X\n");
  (void) printf(
    "resources as command line options:  -background, -bordercolor,\n");
  (void) printf(
    "-borderwidth, -font, -foreground, -iconGeometry, -iconic, -name,\n");
  (void) printf("-mattecolor, -shared-memory, or -title.\n");
  (void) printf(
    "\nBy default, the image format of `file' is determined by its magic\n");
  (void) printf(
    "number.  To specify a particular image format, precede the filename\n");
  (void) printf(
    "with an image format name and a colon (i.e. ps:image) or specify the\n");
  (void) printf(
    "image type as the filename suffix (i.e. image.ps).  Specify 'file' as\n");
  (void) printf("'-' for standard input or output.\n");
  (void) printf("\nButtons: \n");
  for (p=buttons; *p != (char *) NULL; p++)
    (void) printf("  %s\n",*p);
  return(MagickFalse);
}

WandExport MagickBooleanType AnimateImageCommand(ImageInfo *image_info,
  int argc,char **argv,char **wand_unused(metadata),ExceptionInfo *exception)
{
#if defined(MAGICKCORE_X11_DELEGATE)
#define DestroyAnimate() \
{ \
  XDestroyResourceInfo(&resource_info); \
  if (display != (Display *) NULL) \
    { \
      XCloseDisplay(display); \
      display=(Display *) NULL; \
    } \
  XDestroyResourceInfo(&resource_info); \
  DestroyImageStack(); \
  for (i=0; i < (ssize_t) argc; i++) \
    argv[i]=DestroyString(argv[i]); \
  argv=(char **) RelinquishMagickMemory(argv); \
}
#define ThrowAnimateException(asperity,tag,option) \
{ \
  (void) ThrowMagickException(exception,GetMagickModule(),asperity,tag,"`%s'", \
    option); \
  DestroyAnimate(); \
  return(MagickFalse); \
}
#define ThrowAnimateInvalidArgumentException(option,argument) \
{ \
  (void) ThrowMagickException(exception,GetMagickModule(),OptionError, \
    "InvalidArgument","`%s': %s",option,argument); \
  DestroyAnimate(); \
  return(MagickFalse); \
}

  char
    *resource_value,
    *server_name;

  const char
    *option;

  Display
    *display;

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

  QuantizeInfo
    *quantize_info;

  register ssize_t
    i;

  ssize_t
    j,
    k;

  XResourceInfo
    resource_info;

  XrmDatabase
    resource_database;

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
  status=MagickTrue;
  SetNotifyHandlers;
  display=(Display *) NULL;
  j=1;
  k=0;
  NewImageStack();
  option=(char *) NULL;
  pend=MagickFalse;
  respect_parenthesis=MagickFalse;
  resource_database=(XrmDatabase) NULL;
  (void) ResetMagickMemory(&resource_info,0,sizeof(XResourceInfo));
  server_name=(char *) NULL;
  status=MagickTrue;
  /*
    Check for server name specified on the command line.
  */
  ReadCommandlLine(argc,&argv);
  status=ExpandFilenames(&argc,&argv);
  if (status == MagickFalse)
    ThrowAnimateException(ResourceLimitError,"MemoryAllocationFailed",
      image_info->filename);
  for (i=1; i < (ssize_t) argc; i++)
  {
    /*
      Check command line for server name.
    */
    option=argv[i];
    if (LocaleCompare("display",option+1) == 0)
      {
        /*
          User specified server name.
        */
        i++;
        if (i == (ssize_t) argc)
          ThrowAnimateException(OptionError,"MissingArgument",option);
        server_name=argv[i];
      }
    if ((LocaleCompare("help",option+1) == 0) ||
        (LocaleCompare("-help",option+1) == 0))
      return(AnimateUsage());
  }
  /*
    Get user defaults from X resource database.
  */
  display=XOpenDisplay(server_name);
  if (display == (Display *) NULL)
    ThrowAnimateException(XServerError,"UnableToOpenXServer",
      XDisplayName(server_name));
  (void) XSetErrorHandler(XError);
  resource_database=XGetResourceDatabase(display,GetClientName());
  XGetResourceInfo(image_info,resource_database,GetClientName(),
    &resource_info);
  quantize_info=resource_info.quantize_info;
  image_info->density=XGetResourceInstance(resource_database,GetClientName(),
    "density",(char *) NULL);
  if (image_info->density == (char *) NULL)
    image_info->density=XGetScreenDensity(display);
  resource_value=XGetResourceInstance(resource_database,GetClientName(),
    "interlace","none");
  image_info->interlace=(InterlaceType)
    ParseCommandOption(MagickInterlaceOptions,MagickFalse,resource_value);
  resource_value=XGetResourceInstance(resource_database,GetClientName(),
    "verbose","False");
  image_info->verbose=IsMagickTrue(resource_value);
  resource_value=XGetResourceInstance(resource_database,GetClientName(),
    "dither","True");
  quantize_info->dither=IsMagickTrue(resource_value);
  /*
    Parse command line.
  */
  for (i=1; i <= (ssize_t) argc; i++)
  {
    if (i < (ssize_t) argc)
      option=argv[i];
    else
      if (image != (Image *) NULL)
        break;
      else
        if (isatty(STDIN_FILENO) != MagickFalse)
          option="logo:";
        else
          {
            int
              c;

            c=getc(stdin);
            if (c == EOF)
              option="logo:";
            else
              {
                c=ungetc(c,stdin);
                option="-";
              }
          }
    if (LocaleCompare(option,"(") == 0)
      {
        FireImageStack(MagickFalse,MagickTrue,pend);
        if (k == MaxImageStackDepth)
          ThrowAnimateException(OptionError,"ParenthesisNestedTooDeeply",
            option);
        PushImageStack();
        continue;
      }
    if (LocaleCompare(option,")") == 0)
      {
        FireImageStack(MagickFalse,MagickTrue,MagickTrue);
        if (k == 0)
          ThrowAnimateException(OptionError,"UnableToParseExpression",option);
        PopImageStack();
        continue;
      }
    if (IsCommandOption(option) == MagickFalse)
      {
        const char
          *filename;

        Image
          *images;

        /*
          Read input image.
        */
        FireImageStack(MagickFalse,MagickFalse,pend);
        filename=option;
        if ((LocaleCompare(filename,"--") == 0) && (i < (ssize_t) (argc-1)))
          {
            option=argv[++i];
            filename=option;
          }
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
        if (LocaleCompare("alpha",option+1) == 0)
          {
            ssize_t
              type;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowAnimateException(OptionError,"MissingArgument",option);
            type=ParseCommandOption(MagickAlphaOptions,MagickFalse,argv[i]);
            if (type < 0)
              ThrowAnimateException(OptionError,"UnrecognizedAlphaChannelType",
                argv[i]);
            break;
          }
        if (LocaleCompare("authenticate",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowAnimateException(OptionError,"MissingArgument",option);
            break;
          }
        ThrowAnimateException(OptionError,"UnrecognizedOption",option);
      }
      case 'b':
      {
        if (LocaleCompare("backdrop",option+1) == 0)
          {
            resource_info.backdrop=(*option == '-') ? MagickTrue : MagickFalse;
            break;
          }
        if (LocaleCompare("background",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowAnimateException(OptionError,"MissingArgument",option);
            resource_info.background_color=argv[i];
            break;
          }
        if (LocaleCompare("bordercolor",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowAnimateException(OptionError,"MissingArgument",option);
            resource_info.border_color=argv[i];
            break;
          }
        if (LocaleCompare("borderwidth",option+1) == 0)
          {
            resource_info.border_width=0;
            if (*option == '+')
              break;
            i++;
            if ((i == (ssize_t) argc) || (IsGeometry(argv[i]) == MagickFalse))
              ThrowAnimateException(OptionError,"MissingArgument",option);
            resource_info.border_width=(unsigned int)
              StringToUnsignedLong(argv[i]);
            break;
          }
        ThrowAnimateException(OptionError,"UnrecognizedOption",option);
      }
      case 'c':
      {
        if (LocaleCompare("cache",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowAnimateException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowAnimateInvalidArgumentException(option,argv[i]);
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
              ThrowAnimateException(OptionError,"MissingArgument",option);
            channel=ParseChannelOption(argv[i]);
            if (channel < 0)
              ThrowAnimateException(OptionError,"UnrecognizedChannelType",
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
              ThrowAnimateException(ImageError,"ImageSequenceRequired",option);
            FireImageStack(MagickFalse,MagickTrue,MagickTrue);
            if (*option == '+')
              clone_images=CloneImages(clone_images,"-1",exception);
            else
              {
                i++;
                if (i == (ssize_t) (argc-1))
                  ThrowAnimateException(OptionError,"MissingArgument",option);
                if (IsSceneGeometry(argv[i],MagickFalse) == MagickFalse)
                  ThrowAnimateInvalidArgumentException(option,argv[i]);
                clone_images=CloneImages(clone_images,argv[i],exception);
              }
            if (clone_images == (Image *) NULL)
              ThrowAnimateException(OptionError,"NoSuchImage",option);
            AppendImageStack(clone_images);
            break;
          }
        if (LocaleCompare("coalesce",option+1) == 0)
          break;
        if (LocaleCompare("colormap",option+1) == 0)
          {
            resource_info.colormap=PrivateColormap;
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowAnimateException(OptionError,"MissingArgument",option);
            resource_info.colormap=UndefinedColormap;
            if (LocaleCompare("private",argv[i]) == 0)
              resource_info.colormap=PrivateColormap;
            if (LocaleCompare("shared",argv[i]) == 0)
              resource_info.colormap=SharedColormap;
            if (resource_info.colormap == UndefinedColormap)
              ThrowAnimateException(OptionError,"UnrecognizedColormapType",
                argv[i]);
            break;
          }
        if (LocaleCompare("colors",option+1) == 0)
          {
            quantize_info->number_colors=0;
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowAnimateException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowAnimateInvalidArgumentException(option,argv[i]);
            quantize_info->number_colors=StringToUnsignedLong(argv[i]);
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
              ThrowAnimateException(OptionError,"MissingArgument",option);
            colorspace=ParseCommandOption(MagickColorspaceOptions,
              MagickFalse,argv[i]);
            if (colorspace < 0)
              ThrowAnimateException(OptionError,"UnrecognizedColorspace",
                argv[i]);
            break;
          }
        if (LocaleCompare("concurrent",option+1) == 0)
          break;
        if (LocaleCompare("crop",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowAnimateException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowAnimateInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowAnimateException(OptionError,"UnrecognizedOption",option);
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
              ThrowAnimateException(OptionError,"MissingArgument",option);
            event=ParseCommandOption(MagickLogEventOptions,MagickFalse,argv[i]);
            if (event < 0)
              ThrowAnimateException(OptionError,"UnrecognizedEventType",
                argv[i]);
            (void) SetLogEventMask(argv[i]);
            break;
          }
        if (LocaleCompare("decipher",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowAnimateException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("define",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowAnimateException(OptionError,"MissingArgument",option);
            if (*option == '+')
              {
                const char
                  *define;

                define=GetImageOption(image_info,argv[i]);
                if (define == (const char *) NULL)
                  ThrowAnimateException(OptionError,"NoSuchOption",argv[i]);
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
              ThrowAnimateException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowAnimateInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("density",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowAnimateException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowAnimateInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("depth",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowAnimateException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowAnimateInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("display",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowAnimateException(OptionError,"MissingArgument",option);
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
              ThrowAnimateException(OptionError,"MissingArgument",option);
            dispose=ParseCommandOption(MagickDisposeOptions,MagickFalse,argv[i]);
            if (dispose < 0)
              ThrowAnimateException(OptionError,"UnrecognizedDisposeMethod",
                argv[i]);
            break;
          }
        if (LocaleCompare("dither",option+1) == 0)
          {
            ssize_t
              method;

            quantize_info->dither=MagickFalse;
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowAnimateException(OptionError,"MissingArgument",option);
            method=ParseCommandOption(MagickDitherOptions,MagickFalse,argv[i]);
            if (method < 0)
              ThrowAnimateException(OptionError,"UnrecognizedDitherMethod",
                argv[i]);
            quantize_info->dither=MagickTrue;
            quantize_info->dither_method=(DitherMethod) method;
            break;
          }
        if (LocaleCompare("duration",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowAnimateException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowAnimateInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowAnimateException(OptionError,"UnrecognizedOption",option);
      }
      case 'e':
      {
        if (LocaleCompare("extract",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowAnimateException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowAnimateInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowAnimateException(OptionError,"UnrecognizedOption",option);
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
            if (i == (ssize_t) (argc-1))
              ThrowAnimateException(OptionError,"MissingArgument",option);
            filter=ParseCommandOption(MagickFilterOptions,MagickFalse,argv[i]);
            if (filter < 0)
              ThrowAnimateException(OptionError,"UnrecognizedImageFilter",
                argv[i]);
            break;
          }
        if (LocaleCompare("flatten",option+1) == 0)
          break;
        if (LocaleCompare("font",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowAnimateException(OptionError,"MissingArgument",option);
            resource_info.font=XGetResourceClass(resource_database,
              GetClientName(),"font",argv[i]);
            break;
          }
        if (LocaleCompare("foreground",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowAnimateException(OptionError,"MissingArgument",option);
            resource_info.foreground_color=argv[i];
            break;
          }
        if (LocaleCompare("format",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowAnimateException(OptionError,"MissingArgument",option);
            break;
          }
        ThrowAnimateException(OptionError,"UnrecognizedOption",option);
      }
      case 'g':
      {
        if (LocaleCompare("gamma",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowAnimateException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowAnimateInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("geometry",option+1) == 0)
          {
            resource_info.image_geometry=(char *) NULL;
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowAnimateException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowAnimateInvalidArgumentException(option,argv[i]);
            resource_info.image_geometry=ConstantString(argv[i]);
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
              ThrowAnimateException(OptionError,"MissingArgument",option);
            gravity=ParseCommandOption(MagickGravityOptions,MagickFalse,
              argv[i]);
            if (gravity < 0)
              ThrowAnimateException(OptionError,"UnrecognizedGravityType",
                argv[i]);
            break;
          }
        ThrowAnimateException(OptionError,"UnrecognizedOption",option);
      }
      case 'h':
      {
        if ((LocaleCompare("help",option+1) == 0) ||
            (LocaleCompare("-help",option+1) == 0))
          break;
        ThrowAnimateException(OptionError,"UnrecognizedOption",option);
      }
      case 'i':
      {
        if (LocaleCompare("iconGeometry",option+1) == 0)
          {
            resource_info.icon_geometry=(char *) NULL;
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowAnimateException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowAnimateInvalidArgumentException(option,argv[i]);
            resource_info.icon_geometry=argv[i];
            break;
          }
        if (LocaleCompare("iconic",option+1) == 0)
          {
            resource_info.iconic=(*option == '-') ? MagickTrue : MagickFalse;
            break;
          }
        if (LocaleCompare("identify",option+1) == 0)
          break;
        if (LocaleCompare("immutable",option+1) == 0)
          {
            resource_info.immutable=(*option == '-') ? MagickTrue : MagickFalse;
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
              ThrowAnimateException(OptionError,"MissingArgument",option);
            interlace=ParseCommandOption(MagickInterlaceOptions,MagickFalse,
              argv[i]);
            if (interlace < 0)
              ThrowAnimateException(OptionError,"UnrecognizedInterlaceType",
                argv[i]);
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
              ThrowAnimateException(OptionError,"MissingArgument",option);
            interpolate=ParseCommandOption(MagickInterpolateOptions,MagickFalse,
              argv[i]);
            if (interpolate < 0)
              ThrowAnimateException(OptionError,"UnrecognizedInterpolateMethod",
                argv[i]);
            break;
          }
        ThrowAnimateException(OptionError,"UnrecognizedOption",option);
      }
      case 'l':
      {
        if (LocaleCompare("label",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowAnimateException(OptionError,"MissingArgument",option);
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
              ThrowAnimateException(OptionError,"MissingArgument",option);
            resource=ParseCommandOption(MagickResourceOptions,MagickFalse,
              argv[i]);
            if (resource < 0)
              ThrowAnimateException(OptionError,"UnrecognizedResourceType",
                argv[i]);
            i++;
            if (i == (ssize_t) argc)
              ThrowAnimateException(OptionError,"MissingArgument",option);
            value=StringToDouble(argv[i],&p);
            (void) value;
            if ((p == argv[i]) && (LocaleCompare("unlimited",argv[i]) != 0))
              ThrowAnimateInvalidArgumentException(option,argv[i]);
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
              ThrowAnimateException(OptionError,"MissingArgument",option);
            list=ParseCommandOption(MagickListOptions,MagickFalse,argv[i]);
            if (list < 0)
              ThrowAnimateException(OptionError,"UnrecognizedListType",argv[i]);
            status=MogrifyImageInfo(image_info,(int) (i-j+1),(const char **)
              argv+j,exception);
            DestroyAnimate();
            return(status != 0 ? MagickFalse : MagickTrue);
          }
        if (LocaleCompare("log",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if ((i == (ssize_t) argc) ||
                (strchr(argv[i],'%') == (char *) NULL))
              ThrowAnimateException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("loop",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowAnimateException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowAnimateInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowAnimateException(OptionError,"UnrecognizedOption",option);
      }
      case 'm':
      {
        if (LocaleCompare("map",option+1) == 0)
          {
            resource_info.map_type=(char *) NULL;
            if (*option == '+')
              break;
            (void) CopyMagickString(argv[i]+1,"san",MaxTextExtent);
            i++;
            if (i == (ssize_t) argc)
              ThrowAnimateException(OptionError,"MissingArgument",option);
            resource_info.map_type=argv[i];
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
              ThrowAnimateException(OptionError,"MissingArgument",option);
            resource_info.matte_color=argv[i];
            break;
          }
        if (LocaleCompare("monitor",option+1) == 0)
          break;
        if (LocaleCompare("monochrome",option+1) == 0)
          {
            if (*option == '+')
              break;
            quantize_info->number_colors=2;
            quantize_info->colorspace=GRAYColorspace;
            break;
          }
        ThrowAnimateException(OptionError,"UnrecognizedOption",option);
      }
      case 'n':
      {
        if (LocaleCompare("name",option+1) == 0)
          {
            resource_info.name=(char *) NULL;
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowAnimateException(OptionError,"MissingArgument",option);
            resource_info.name=ConstantString(argv[i]);
            break;
          }
        if (LocaleCompare("noop",option+1) == 0)
          break;
        ThrowAnimateException(OptionError,"UnrecognizedOption",option);
      }
      case 'p':
      {
        if (LocaleCompare("pause",option+1) == 0)
          {
            resource_info.pause=0;
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowAnimateException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowAnimateInvalidArgumentException(option,argv[i]);
            resource_info.pause=(unsigned int) StringToUnsignedLong(argv[i]);
            break;
          }
        if (LocaleCompare("page",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowAnimateException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("profile",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowAnimateException(OptionError,"MissingArgument",option);
            break;
          }
        ThrowAnimateException(OptionError,"UnrecognizedOption",option);
      }
      case 'q':
      {
        if (LocaleCompare("quantize",option+1) == 0)
          {
            ssize_t
              colorspace;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowAnimateException(OptionError,"MissingArgument",option);
            colorspace=ParseCommandOption(MagickColorspaceOptions,
              MagickFalse,argv[i]);
            if (colorspace < 0)
              ThrowAnimateException(OptionError,"UnrecognizedColorspace",
                argv[i]);
            break;
          }
        if (LocaleCompare("quiet",option+1) == 0)
          break;
        ThrowAnimateException(OptionError,"UnrecognizedOption",option);
      }
      case 'r':
      {
        if (LocaleCompare("regard-warnings",option+1) == 0)
          break;
        if (LocaleCompare("remote",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowAnimateException(OptionError,"MissingArgument",option);
            if (XRemoteCommand(display,resource_info.window_id,argv[i]) != 0)
              return(MagickFalse);
            i--;
            break;
          }
        if (LocaleCompare("repage",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowAnimateException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowAnimateInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("resample",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowAnimateException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowAnimateInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("resize",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowAnimateException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowAnimateInvalidArgumentException(option,argv[i]);
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
            if (i == (ssize_t) argc)
              ThrowAnimateException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowAnimateInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowAnimateException(OptionError,"UnrecognizedOption",option);
      }
      case 's':
      {
        if (LocaleCompare("sampling-factor",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowAnimateException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowAnimateInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("seed",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowAnimateException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowAnimateInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("scenes",option+1) == 0)  /* deprecated */
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowAnimateException(OptionError,"MissingArgument",option);
            if (IsSceneGeometry(argv[i],MagickFalse) == MagickFalse)
              ThrowAnimateInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("set",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowAnimateException(OptionError,"MissingArgument",option);
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowAnimateException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("shared-memory",option+1) == 0)
          {
            resource_info.use_shared_memory=(*option == '-') ? MagickTrue :
              MagickFalse;
            break;
          }
        if (LocaleCompare("size",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowAnimateException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowAnimateInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("strip",option+1) == 0)
          break;
        if (LocaleCompare("support",option+1) == 0)
          {
            i++;  /* deprecated */
            break;
          }
        ThrowAnimateException(OptionError,"UnrecognizedOption",option);
      }
      case 't':
      {
        if (LocaleCompare("text-font",option+1) == 0)
          {
            resource_info.text_font=(char *) NULL;
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowAnimateException(OptionError,"MissingArgument",option);
            resource_info.text_font=XGetResourceClass(resource_database,
              GetClientName(),"font",argv[i]);
            break;
          }
        if (LocaleCompare("thumbnail",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowAnimateException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowAnimateInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("title",option+1) == 0)
          {
            resource_info.title=(char *) NULL;
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowAnimateException(OptionError,"MissingArgument",option);
            resource_info.title=argv[i];
            break;
          }
        if (LocaleCompare("transparent-color",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) (argc-1))
              ThrowAnimateException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("treedepth",option+1) == 0)
          {
            quantize_info->tree_depth=0;
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowAnimateException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowAnimateInvalidArgumentException(option,argv[i]);
            quantize_info->tree_depth=StringToUnsignedLong(argv[i]);
            break;
          }
        if (LocaleCompare("trim",option+1) == 0)
          break;
        ThrowAnimateException(OptionError,"UnrecognizedOption",option);
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
        if (LocaleCompare("virtual-pixel",option+1) == 0)
          {
            ssize_t
              method;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowAnimateException(OptionError,"MissingArgument",option);
            method=ParseCommandOption(MagickVirtualPixelOptions,MagickFalse,
              argv[i]);
            if (method < 0)
              ThrowAnimateException(OptionError,
                "UnrecognizedVirtualPixelMethod",argv[i]);
            break;
          }
        if (LocaleCompare("visual",option+1) == 0)
          {
            resource_info.visual_type=(char *) NULL;
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowAnimateException(OptionError,"MissingArgument",option);
            resource_info.visual_type=argv[i];
            break;
          }
        ThrowAnimateException(OptionError,"UnrecognizedOption",option);
      }
      case 'w':
      {
        if (LocaleCompare("window",option+1) == 0)
          {
            resource_info.window_id=(char *) NULL;
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowAnimateException(OptionError,"MissingArgument",option);
            resource_info.window_id=argv[i];
            break;
          }
        ThrowAnimateException(OptionError,"UnrecognizedOption",option);
      }
      case '?':
        break;
      default:
        ThrowAnimateException(OptionError,"UnrecognizedOption",option);
    }
    fire=(GetCommandOptionFlags(MagickCommandOptions,MagickFalse,option) &
      FireOptionFlag) == 0 ?  MagickFalse : MagickTrue;
    if (fire != MagickFalse)
      FireImageStack(MagickFalse,MagickTrue,MagickTrue);
  }
  i--;
  if (k != 0)
    ThrowAnimateException(OptionError,"UnbalancedParenthesis",argv[i]);
  if (image == (Image *) NULL)
    ThrowAnimateException(OptionError,"MissingAnImageFilename",argv[argc-1])
  FinalizeImageSettings(image_info,image,MagickTrue);
  if (image == (Image *) NULL)
    ThrowAnimateException(OptionError,"MissingAnImageFilename",argv[argc-1])
  if (resource_info.window_id != (char *) NULL)
    {
      XAnimateBackgroundImage(display,&resource_info,image);
      status&=MagickTrue;
    }
  else
    {
      Image
        *animate_image;

      /*
        Animate image to X server.
      */
      animate_image=XAnimateImages(display,&resource_info,argv,argc,image);
      status&=animate_image != (Image *) NULL;
      while (animate_image != (Image *) NULL)
      {
        image=animate_image;
        animate_image=XAnimateImages(display,&resource_info,argv,argc,image);
        if (animate_image != image)
          image=DestroyImageList(image);
      }
    }
  DestroyAnimate();
  return(status != 0 ? MagickTrue : MagickFalse);
#else
  (void) argc;
  (void) argv;
  (void) ThrowMagickException(exception,GetMagickModule(),MissingDelegateError,
    "DelegateLibrarySupportNotBuiltIn","`%s' (X11)",image_info->filename);
  return(AnimateUsage());
#endif
}
