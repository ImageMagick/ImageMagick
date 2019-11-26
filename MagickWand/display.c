/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%               DDDD   IIIII  SSSSS  PPPP   L       AAA   Y   Y               %
%               D   D    I    SS     P   P  L      A   A   Y Y                %
%               D   D    I     SSS   PPPP   L      AAAAA    Y                 %
%               D   D    I       SS  P      L      A   A    Y                 %
%               DDDD   IIIII  SSSSS  P      LLLLL  A   A    Y                 %
%                                                                             %
%                                                                             %
%              Methods to Interactively Display and Edit an Image             %
%                                                                             %
%                             Software Design                                 %
%                                  Cristy                                     %
%                                July 1992                                    %
%                                                                             %
%                                                                             %
%  Copyright 1999-2020 ImageMagick Studio LLC, a non-profit organization      %
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
%  Use the display program to display an image or image sequence on any X
%  server.
%
*/

/*
  Include declarations.
*/
#include "MagickWand/studio.h"
#include "MagickWand/MagickWand.h"
#include "MagickWand/mogrify-private.h"
#include "MagickCore/display-private.h"
#include "MagickCore/string-private.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   D i s p l a y I m a g e C o m m a n d                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  DisplayImageCommand() displays a sequence of images on any workstation
%  display running an X server.  Display first determines the hardware
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
%  The format of the DisplayImageCommand method is:
%
%      MagickBooleanType DisplayImageCommand(ImageInfo *image_info,int argc,
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

static MagickBooleanType DisplayUsage(void)
{
  static const char
    buttons[] =
      "  1    press to map or unmap the Command widget\n"
      "  2    press and drag to magnify a region of an image\n"
      "  3    press to load an image from a visual image directory",
    miscellaneous[] =
      "  -debug events        display copious debugging information\n"
      "  -help                print program options\n"
      "  -list type           print a list of supported option arguments\n"
      "  -log format          format of debugging information\n"
      "  -version             print version information",
    operators[] =
      "  -auto-orient         automagically orient image\n"
      "  -border geometry     surround image with a border of color\n"
      "  -clip                clip along the first path from the 8BIM profile\n"
      "  -clip-path id        clip along a named path from the 8BIM profile\n"
      "  -colors value        preferred number of colors in the image\n"
      "  -contrast            enhance or reduce the image contrast\n"
      "  -crop geometry       preferred size and location of the cropped image\n"
      "  -decipher filename   convert cipher pixels to plain pixels\n"
      "  -deskew threshold    straighten an image\n"
      "  -despeckle           reduce the speckles within an image\n"
      "  -edge factor         apply a filter to detect edges in the image\n"
      "  -enhance             apply a digital filter to enhance a noisy image\n"
      "  -equalize            perform histogram equalization to an image\n"
      "  -extract geometry    extract area from image\n"
      "  -flip                flip image in the vertical direction\n"
      "  -flop                flop image in the horizontal direction\n"
      "  -frame geometry      surround image with an ornamental border\n"
      "  -fuzz distance       colors within this distance are considered equal\n"
      "  -gamma value         level of gamma correction\n"
      "  -monochrome          transform image to black and white\n"
      "  -negate              replace every pixel with its complementary color\n"
      "  -normalize           transform image to span the full range of colors\n"
      "  -raise value         lighten/darken image edges to create a 3-D effect\n"
      "  -resample geometry   change the resolution of an image\n"
      "  -resize geometry     resize the image\n"
      "  -roll geometry       roll an image vertically or horizontally\n"
      "  -rotate degrees      apply Paeth rotation to the image\n"
      "  -sample geometry     scale image with pixel sampling\n"
      "  -segment value       segment an image\n"
      "  -sharpen geometry    sharpen the image\n"
      "  -strip               strip image of all profiles and comments\n"
      "  -threshold value     threshold the image\n"
      "  -thumbnail geometry  create a thumbnail of the image\n"
      "  -trim                trim image edges",
    settings[] =
      "  -alpha option        on, activate, off, deactivate, set, opaque, copy\n"
      "                       transparent, extract, background, or shape\n"
      "  -antialias           remove pixel-aliasing\n"
      "  -authenticate password\n"
      "                       decipher image with this password\n"
      "  -backdrop            display image centered on a backdrop\n"
      "  -channel type        apply option to select image channels\n"
      "  -colormap type       Shared or Private\n"
      "  -colorspace type     alternate image colorspace\n"
      "  -comment string      annotate image with comment\n"
      "  -compress type       type of pixel compression when writing the image\n"
      "  -define format:option\n"
      "                       define one or more image format options\n"
      "  -delay value         display the next image after pausing\n"
      "  -density geometry    horizontal and vertical density of the image\n"
      "  -depth value         image depth\n"
      "  -display server      display image to this X server\n"
      "  -dispose method      layer disposal method\n"
      "  -dither method       apply error diffusion to image\n"
      "  -endian type         endianness (MSB or LSB) of the image\n"
      "  -filter type         use this filter when resizing an image\n"
      "  -format string     output formatted image characteristics\n"
      "  -geometry geometry   preferred size and location of the Image window\n"
      "  -gravity type        horizontal and vertical backdrop placement\n"
      "  -identify            identify the format and characteristics of the image\n"
      "  -immutable           displayed image cannot be modified\n"
      "  -interlace type      type of image interlacing scheme\n"
      "  -interpolate method  pixel color interpolation method\n"
      "  -label string        assign a label to an image\n"
      "  -limit type value    pixel cache resource limit\n"
      "  -loop iterations     loop images then exit\n"
      "  -map type            display image using this Standard Colormap\n"
      "  -matte               store matte channel if the image has one\n"
      "  -monitor             monitor progress\n"
      "  -nostdin             do not try to open stdin\n"
      "  -page geometry       size and location of an image canvas\n"
      "  -profile filename    add, delete, or apply an image profile\n"
      "  -quality value       JPEG/MIFF/PNG compression level\n"
      "  -quantize colorspace reduce colors in this colorspace\n"
      "  -quiet               suppress all warning messages\n"
      "  -regard-warnings     pay attention to warning messages\n"
      "  -remote command      execute a command in an remote display process\n"
      "  -repage geometry     size and location of an image canvas (operator)\n"
      "  -respect-parentheses settings remain in effect until parenthesis boundary\n"
      "  -sampling-factor geometry\n"
      "                       horizontal and vertical sampling factor\n"
      "  -scenes range        image scene range\n"
      "  -seed value          seed a new sequence of pseudo-random numbers\n"
      "  -set property value  set an image property\n"
      "  -size geometry       width and height of image\n"
      "  -support factor      resize support: > 1.0 is blurry, < 1.0 is sharp\n"
      "  -texture filename    name of texture to tile onto the image background\n"
      "  -transparent-color color\n"
      "                       transparent color\n"
      "  -treedepth value     color tree depth\n"
      "  -update seconds      detect when image file is modified and redisplay\n"
      "  -verbose             print detailed information about the image\n"
      "  -visual type         display image using this visual type\n"
      "  -virtual-pixel method\n"
      "                       virtual pixel access method\n"
      "  -window id           display image to background of this window\n"
      "  -window-group id     exit program when this window id is destroyed\n"
      "  -write filename      write image to a file",
    sequence_operators[] =
      "  -coalesce            merge a sequence of images\n"
      "  -flatten             flatten a sequence of images";

  ListMagickVersion(stdout);
  (void) printf("Usage: %s [options ...] file [ [options ...] file ...]\n",
    GetClientName());
  (void) printf("\nImage Settings:\n");
  (void) puts(settings);
  (void) printf("\nImage Operators:\n");
  (void) puts(operators);
  (void) printf("\nImage Sequence Operators:\n");
  (void) puts(sequence_operators);
  (void) printf("\nMiscellaneous Options:\n");
  (void) puts(miscellaneous);
  (void) printf(
    "\nIn addition to those listed above, you can specify these standard X\n");
  (void) printf(
    "resources as command line options:  -background, -bordercolor,\n");
  (void) printf(
    " -mattecolor, -borderwidth, -font, -foreground, -iconGeometry,\n");
  (void) printf("-iconic, -name, -shared-memory, -usePixmap, or -title.\n");
  (void) printf(
    "\nBy default, the image format of 'file' is determined by its magic\n");
  (void) printf(
    "number.  To specify a particular image format, precede the filename\n");
  (void) printf(
    "with an image format name and a colon (i.e. ps:image) or specify the\n");
  (void) printf(
    "image type as the filename suffix (i.e. image.ps).  Specify 'file' as\n");
  (void) printf("'-' for standard input or output.\n");
  (void) printf("\nButtons: \n");
  (void) puts(buttons);
  return(MagickFalse);
}

WandExport MagickBooleanType DisplayImageCommand(ImageInfo *image_info,
  int argc,char **argv,char **wand_unused(metadata),ExceptionInfo *exception)
{
#if defined(MAGICKCORE_X11_DELEGATE)
#define DestroyDisplay() \
{ \
  if ((state & ExitState) == 0) \
    DestroyXResources(); \
  if (display != (Display *) NULL) \
    { \
      XCloseDisplay(display); \
      display=(Display *) NULL; \
    } \
  XDestroyResourceInfo(&resource_info); \
  DestroyImageStack(); \
  if (image_marker != (size_t *) NULL) \
    image_marker=(size_t *) RelinquishMagickMemory(image_marker); \
  for (i=0; i < (ssize_t) argc; i++) \
    argv[i]=DestroyString(argv[i]); \
  argv=(char **) RelinquishMagickMemory(argv); \
}
#define ThrowDisplayException(asperity,tag,option) \
{ \
  (void) ThrowMagickException(exception,GetMagickModule(),asperity,tag,"`%s'", \
    option); \
  DestroyDisplay(); \
  return(MagickFalse); \
}
#define ThrowDisplayInvalidArgumentException(option,argument) \
{ \
  (void) ThrowMagickException(exception,GetMagickModule(),OptionError, \
    "InvalidArgument","'%s': %s",option,argument); \
  DestroyDisplay(); \
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
    nostdin,
    pend,
    respect_parenthesis;

  MagickStatusType
    status;

  QuantizeInfo
    *quantize_info;

  register ssize_t
    i;

  size_t
    *image_marker,
    iterations,
    last_image,
    state;

  ssize_t
    image_number,
    iteration,
    j,
    k,
    l;

  XResourceInfo
    resource_info;

  XrmDatabase
    resource_database;

  /*
    Set defaults.
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
  SetNotifyHandlers;
  display=(Display *) NULL;
  j=1;
  k=0;
  image_marker=(size_t *) NULL;
  image_number=0;
  last_image=0;
  NewImageStack();
  option=(char *) NULL;
  pend=MagickFalse;
  respect_parenthesis=MagickFalse;
  nostdin=MagickFalse;
  resource_database=(XrmDatabase) NULL;
  (void) memset(&resource_info,0,sizeof(resource_info));
  server_name=(char *) NULL;
  state=0;
  status=MagickTrue;
  ReadCommandlLine(argc,&argv);
  status=ExpandFilenames(&argc,&argv);
  if (status == MagickFalse)
    ThrowDisplayException(ResourceLimitError,"MemoryAllocationFailed",
      GetExceptionMessage(errno));
  image_marker=(size_t *) AcquireQuantumMemory((size_t) argc+1UL,
    sizeof(*image_marker));
  if (image_marker == (size_t *) NULL)
    ThrowDisplayException(ResourceLimitError,"MemoryAllocationFailed",
      GetExceptionMessage(errno));
  for (i=0; i <= (ssize_t) argc; i++)
    image_marker[i]=(size_t) argc;
  /*
    Check for server name specified on the command line.
  */
  for (i=1; i < (ssize_t) argc; i++)
  {
    /*
      Check command line for server name.
    */
    option=argv[i];
    if (IsCommandOption(option) == MagickFalse)
      continue;
    if (LocaleCompare("display",option+1) == 0)
      {
        /*
          User specified server name.
        */
        i++;
        if (i == (ssize_t) argc)
          ThrowDisplayException(OptionError,"MissingArgument",option);
        server_name=argv[i];
      }
    if (LocaleCompare("nostdin",option+1) == 0)
      nostdin=MagickTrue;
    if ((LocaleCompare("help",option+1) == 0) ||
        (LocaleCompare("-help",option+1) == 0))
      return(DisplayUsage());
  }
  /*
    Get user defaults from X resource database.
  */
  display=XOpenDisplay(server_name);
  if (display == (Display *) NULL)
    ThrowDisplayException(XServerError,"UnableToOpenXServer",
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
  image_info->page=XGetResourceInstance(resource_database,GetClientName(),
    "pageGeometry",(char *) NULL);
  resource_value=XGetResourceInstance(resource_database,GetClientName(),
    "quality","75");
  image_info->quality=StringToUnsignedLong(resource_value);
  resource_value=XGetResourceInstance(resource_database,GetClientName(),
    "verbose","False");
  image_info->verbose=IsStringTrue(resource_value);
  resource_value=XGetResourceInstance(resource_database,GetClientName(),
    "dither","True");
  quantize_info->dither_method=IsStringTrue(resource_value) != MagickFalse ?
    RiemersmaDitherMethod : NoDitherMethod;
  /*
    Parse command line.
  */
  iteration=0;
  for (i=1; ((i <= (ssize_t) argc) && ((state & ExitState) == 0)); i++)
  {
    if (i < (ssize_t) argc)
      option=argv[i];
    else
      if (image != (Image *) NULL)
        break;
      else
        if (isatty(STDIN_FILENO) != MagickFalse || (nostdin != MagickFalse))
          option="logo:";
        else
          option="-";
    if (LocaleCompare(option,"(") == 0)
      {
        FireImageStack(MagickFalse,MagickTrue,pend);
        if (k == MaxImageStackDepth)
          ThrowDisplayException(OptionError,"ParenthesisNestedTooDeeply",
            option);
        PushImageStack();
        continue;
      }
    if (LocaleCompare(option,")") == 0)
      {
        FireImageStack(MagickFalse,MagickTrue,MagickTrue);
        if (k == 0)
          ThrowDisplayException(OptionError,"UnableToParseExpression",option);
        PopImageStack();
        continue;
      }
    if (IsCommandOption(option) == MagickFalse)
      {
        const char
          *filename;

        Image
          *display_image,
          *image_list,
          *images;

        /*
          Option is a file name.
        */
        FireImageStack(MagickFalse,MagickFalse,pend);
        filename=option;
        if ((LocaleCompare(filename,"--") == 0) && (i < (ssize_t) (argc-1)))
          {
            option=argv[++i];
            filename=option;
          }
        (void) CopyMagickString(image_info->filename,filename,MagickPathExtent);
        images=ReadImage(image_info,exception);
        CatchException(exception);
        status&=(images != (Image *) NULL) &&
          (exception->severity < ErrorException);
        if (images == (Image *) NULL)
          continue;
        AppendImageStack(images);
        FinalizeImageSettings(image_info,image,MagickFalse);
        iterations=image->iterations;
        image_list=CloneImageList(image,exception);
        if (image_list == (Image *) NULL)
          ThrowDisplayException(ResourceLimitError,"MemoryAllocationFailed",
            GetExceptionMessage(errno));
        display_image=image_list;
        do
        {
          /*
            Transmogrify image as defined by the image processing options.
          */
          resource_info.quantum=1;
          if (resource_info.window_id != (char *) NULL)
            {
              /*
                Display image to a specified X window.
              */
              status=XDisplayBackgroundImage(display,&resource_info,
                display_image,exception);
              if (status != MagickFalse)
                {
                  state|=RetainColorsState;
                  status=MagickFalse;
                }
              if (GetNextImageInList(display_image) == (Image *) NULL)
                state|=ExitState;
            }
          else
            do
            {
              Image
                *nexus;

              /*
                Display image to X server.
              */
              if (resource_info.delay != 1)
                display_image->delay=resource_info.delay;
              nexus=XDisplayImage(display,&resource_info,argv,argc,
                &display_image,&state,exception);
              if (nexus == (Image *) NULL)
                break;
              while ((nexus != (Image *) NULL) && ((state & ExitState) == 0))
              {
                Image
                  *next;

                if (nexus->montage != (char *) NULL)
                  {
                    /*
                      User selected a visual directory image (montage).
                    */
                    display_image=nexus;
                    break;
                  }
                next=XDisplayImage(display,&resource_info,argv,argc,&nexus,
                  &state,exception);
                if ((next == (Image *) NULL) &&
                    (GetNextImageInList(nexus) != (Image *) NULL))
                  {
                    display_image=GetNextImageInList(nexus);
                    nexus=NewImageList();
                  }
                else
                  {
                    if (nexus != display_image)
                      nexus=DestroyImageList(nexus);
                    nexus=next;
                  }
              }
            } while ((state & ExitState) == 0);
          if (resource_info.write_filename != (char *) NULL)
            {
              /*
                Write image.
              */
              (void) CopyMagickString(display_image->filename,
                resource_info.write_filename,MagickPathExtent);
              (void) SetImageInfo(image_info,1,exception);
              status&=WriteImage(image_info,display_image,exception);
            }
          /*
            Proceed to next/previous image.
          */
          if ((state & FormerImageState) != 0)
            for (l=0; l < (ssize_t) resource_info.quantum; l++)
            {
              if (GetPreviousImageInList(display_image) == (Image *) NULL)
                break;
              display_image=GetPreviousImageInList(display_image);
            }
          else
            for (l=0; l < (ssize_t) resource_info.quantum; l++)
            {
              if (GetNextImageInList(display_image) == (Image *) NULL)
                break;
              display_image=GetNextImageInList(display_image);
            }
          if (l < (ssize_t) resource_info.quantum)
            break;
        } while ((display_image != (Image *) NULL) && ((state & ExitState) == 0));
        /*
          Free image resources.
        */
        display_image=DestroyImageList(display_image);
        if ((state & FormerImageState) == 0)
          {
            last_image=(size_t) image_number;
            image_marker[i]=(size_t) image_number++;
          }
        else
          {
            /*
              Proceed to previous image.
            */
            for (i--; i > 0; i--)
              if (image_marker[i] == (size_t) (image_number-2))
                break;
            image_number--;
          }
        if ((i == (ssize_t) argc) && ((state & ExitState) == 0))
          i=0;
        if ((state & ExitState) != 0)
          break;
        /*
          Determine if we should proceed to the first image.
        */
        if (image_number < 0)
          {
            if ((state & FormerImageState) != 0)
              {

                for (i=1; i < (ssize_t) (argc-2); i++)
                  if (last_image == image_marker[i])
                    break;
                image_number=(ssize_t) image_marker[i]+1;
              }
            continue;
          }
        if (resource_info.window_id != (char *) NULL)
          state|=ExitState;
        if (iterations != 0)
          {
            if (++iteration == (ssize_t) iterations)
              state|=ExitState;
            i=0;
          }
        if (LocaleCompare(filename,"-") == 0)
          state|=ExitState;
        RemoveAllImageStack();
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
              ThrowDisplayException(OptionError,"MissingArgument",option);
            type=ParseCommandOption(MagickAlphaChannelOptions,MagickFalse,
              argv[i]);
            if (type < 0)
              ThrowDisplayException(OptionError,
                "UnrecognizedAlphaChannelOption",argv[i]);
            break;
          }
        if (LocaleCompare("antialias",option+1) == 0)
          break;
        if (LocaleCompare("authenticate",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("auto-orient",option+1) == 0)
          break;
        ThrowDisplayException(OptionError,"UnrecognizedOption",option);
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
              ThrowDisplayException(OptionError,"MissingArgument",option);
            resource_info.background_color=argv[i];
            break;
          }
        if (LocaleCompare("border",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowDisplayInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("bordercolor",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            resource_info.border_color=argv[i];
            break;
          }
        if (LocaleCompare("borderwidth",option+1) == 0)
          {
            resource_info.border_width=0;
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowDisplayInvalidArgumentException(option,argv[i]);
            resource_info.border_width=(unsigned int)
              StringToUnsignedLong(argv[i]);
            break;
          }
        ThrowDisplayException(OptionError,"UnrecognizedOption",option);
      }
      case 'c':
      {
        if (LocaleCompare("cache",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowDisplayInvalidArgumentException(option,argv[i]);
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
              ThrowDisplayException(OptionError,"MissingArgument",option);
            channel=ParseChannelOption(argv[i]);
            if (channel < 0)
              ThrowDisplayException(OptionError,"UnrecognizedChannelType",
                argv[i]);
            break;
          }
        if (LocaleCompare("clip",option+1) == 0)
          break;
        if (LocaleCompare("clip-path",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
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
              ThrowDisplayException(OptionError,"MissingArgument",option);
            resource_info.colormap=UndefinedColormap;
            if (LocaleCompare("private",argv[i]) == 0)
              resource_info.colormap=PrivateColormap;
            if (LocaleCompare("shared",argv[i]) == 0)
              resource_info.colormap=SharedColormap;
            if (resource_info.colormap == UndefinedColormap)
              ThrowDisplayException(OptionError,"UnrecognizedColormapType",
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
              ThrowDisplayException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowDisplayInvalidArgumentException(option,argv[i]);
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
              ThrowDisplayException(OptionError,"MissingArgument",option);
            colorspace=ParseCommandOption(MagickColorspaceOptions,
              MagickFalse,argv[i]);
            if (colorspace < 0)
              ThrowDisplayException(OptionError,"UnrecognizedColorspace",
                argv[i]);
            break;
          }
        if (LocaleCompare("comment",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
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
              ThrowDisplayException(OptionError,"MissingArgument",option);
            compress=ParseCommandOption(MagickCompressOptions,MagickFalse,
              argv[i]);
            if (compress < 0)
              ThrowDisplayException(OptionError,"UnrecognizedImageCompression",
                argv[i]);
            break;
          }
        if (LocaleCompare("concurrent",option+1) == 0)
          break;
        if (LocaleCompare("contrast",option+1) == 0)
          break;
        if (LocaleCompare("crop",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowDisplayInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowDisplayException(OptionError,"UnrecognizedOption",option);
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
              ThrowDisplayException(OptionError,"MissingArgument",option);
            event=ParseCommandOption(MagickLogEventOptions,MagickFalse,argv[i]);
            if (event < 0)
              ThrowDisplayException(OptionError,"UnrecognizedEventType",
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
              ThrowDisplayException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("define",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            if (*option == '+')
              {
                const char
                  *define;

                define=GetImageOption(image_info,argv[i]);
                if (define == (const char *) NULL)
                  ThrowDisplayException(OptionError,"NoSuchOption",argv[i]);
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
              ThrowDisplayException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowDisplayInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("density",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowDisplayInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("depth",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowDisplayInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("deskew",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowDisplayInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("despeckle",option+1) == 0)
          break;
        if (LocaleCompare("display",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
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
              ThrowDisplayException(OptionError,"MissingArgument",option);
            dispose=ParseCommandOption(MagickDisposeOptions,MagickFalse,argv[i]);
            if (dispose < 0)
              ThrowDisplayException(OptionError,"UnrecognizedDisposeMethod",
                argv[i]);
            break;
          }
        if (LocaleCompare("dither",option+1) == 0)
          {
            ssize_t
              method;

            quantize_info->dither_method=NoDitherMethod;
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            method=ParseCommandOption(MagickDitherOptions,MagickFalse,argv[i]);
            if (method < 0)
              ThrowDisplayException(OptionError,"UnrecognizedDitherMethod",
                argv[i]);
            quantize_info->dither_method=(DitherMethod) method;
            break;
          }
        if (LocaleCompare("duration",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowDisplayInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowDisplayException(OptionError,"UnrecognizedOption",option);
      }
      case 'e':
      {
        if (LocaleCompare("edge",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowDisplayInvalidArgumentException(option,argv[i]);
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
              ThrowDisplayException(OptionError,"MissingArgument",option);
            endian=ParseCommandOption(MagickEndianOptions,MagickFalse,
              argv[i]);
            if (endian < 0)
              ThrowDisplayException(OptionError,"UnrecognizedEndianType",
                argv[i]);
            break;
          }
        if (LocaleCompare("enhance",option+1) == 0)
          break;
        if (LocaleCompare("equalize",option+1) == 0)
          break;
        if (LocaleCompare("extract",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowDisplayInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowDisplayException(OptionError,"UnrecognizedOption",option);
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
              ThrowDisplayException(OptionError,"MissingArgument",option);
            filter=ParseCommandOption(MagickFilterOptions,MagickFalse,argv[i]);
            if (filter < 0)
              ThrowDisplayException(OptionError,"UnrecognizedImageFilter",
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
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
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
              ThrowDisplayException(OptionError,"MissingArgument",option);
            resource_info.foreground_color=argv[i];
            break;
          }
        if (LocaleCompare("format",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("frame",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowDisplayInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("fuzz",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowDisplayInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowDisplayException(OptionError,"UnrecognizedOption",option);
      }
      case 'g':
      {
        if (LocaleCompare("gamma",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowDisplayInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("geometry",option+1) == 0)
          {
            resource_info.image_geometry=(char *) NULL;
            if (*option == '+')
              break;
            (void) CopyMagickString(argv[i]+1,"sans",MagickPathExtent);
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowDisplayInvalidArgumentException(option,argv[i]);
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
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            gravity=ParseCommandOption(MagickGravityOptions,MagickFalse,
              argv[i]);
            if (gravity < 0)
              ThrowDisplayException(OptionError,"UnrecognizedGravityType",
                argv[i]);
            break;
          }
        ThrowDisplayException(OptionError,"UnrecognizedOption",option);
      }
      case 'h':
      {
        if ((LocaleCompare("help",option+1) == 0) ||
            (LocaleCompare("-help",option+1) == 0))
          break;
        ThrowDisplayException(OptionError,"UnrecognizedOption",option);
      }
      case 'i':
      {
        if (LocaleCompare("identify",option+1) == 0)
          break;
        if (LocaleCompare("iconGeometry",option+1) == 0)
          {
            resource_info.icon_geometry=(char *) NULL;
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowDisplayInvalidArgumentException(option,argv[i]);
            resource_info.icon_geometry=argv[i];
            break;
          }
        if (LocaleCompare("iconic",option+1) == 0)
          {
            resource_info.iconic=(*option == '-') ? MagickTrue : MagickFalse;
            break;
          }
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
              ThrowDisplayException(OptionError,"MissingArgument",option);
            interlace=ParseCommandOption(MagickInterlaceOptions,MagickFalse,
              argv[i]);
            if (interlace < 0)
              ThrowDisplayException(OptionError,"UnrecognizedInterlaceType",
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
              ThrowDisplayException(OptionError,"MissingArgument",option);
            interpolate=ParseCommandOption(MagickInterpolateOptions,MagickFalse,
              argv[i]);
            if (interpolate < 0)
              ThrowDisplayException(OptionError,"UnrecognizedInterpolateMethod",
                argv[i]);
            break;
          }
        ThrowDisplayException(OptionError,"UnrecognizedOption",option);
      }
      case 'l':
      {
        if (LocaleCompare("label",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
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
              ThrowDisplayException(OptionError,"MissingArgument",option);
            resource=ParseCommandOption(MagickResourceOptions,MagickFalse,
              argv[i]);
            if (resource < 0)
              ThrowDisplayException(OptionError,"UnrecognizedResourceType",
                argv[i]);
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            value=StringToDouble(argv[i],&p);
            (void) value;
            if ((p == argv[i]) && (LocaleCompare("unlimited",argv[i]) != 0))
              ThrowDisplayInvalidArgumentException(option,argv[i]);
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
              ThrowDisplayException(OptionError,"MissingArgument",option);
            list=ParseCommandOption(MagickListOptions,MagickFalse,argv[i]);
            if (list < 0)
              ThrowDisplayException(OptionError,"UnrecognizedListType",argv[i]);
            status=MogrifyImageInfo(image_info,(int) (i-j+1),(const char **)
              argv+j,exception);
            DestroyDisplay();
            return(status == 0 ? MagickFalse : MagickTrue);
          }
        if (LocaleCompare("log",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if ((i == (ssize_t) argc) ||
                (strchr(argv[i],'%') == (char *) NULL))
              ThrowDisplayException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("loop",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowDisplayInvalidArgumentException(option,argv[i]);
            iterations=StringToUnsignedLong(argv[i]);
            break;
          }
        ThrowDisplayException(OptionError,"UnrecognizedOption",option);
      }
      case 'm':
      {
        if (LocaleCompare("magnify",option+1) == 0)
          {
            resource_info.magnify=2;
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowDisplayInvalidArgumentException(option,argv[i]);
            resource_info.magnify=(unsigned int) StringToUnsignedLong(argv[i]);
            break;
          }
        if (LocaleCompare("map",option+1) == 0)
          {
            resource_info.map_type=(char *) NULL;
            if (*option == '+')
              break;
            (void) strcpy(argv[i]+1,"san");
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
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
              ThrowDisplayException(OptionError,"MissingArgument",option);
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
        ThrowDisplayException(OptionError,"UnrecognizedOption",option);
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
              ThrowDisplayException(OptionError,"MissingArgument",option);
            resource_info.name=ConstantString(argv[i]);
            break;
          }
        if (LocaleCompare("negate",option+1) == 0)
          break;
        if (LocaleCompare("noop",option+1) == 0)
          break;
        if (LocaleCompare("normalize",option+1) == 0)
          break;
        if (LocaleCompare("nostdin",option+1) == 0)
          break;
        ThrowDisplayException(OptionError,"UnrecognizedOption",option);
      }
      case 'p':
      {
        if (LocaleCompare("page",option+1) == 0)
          {
            resource_info.image_geometry=(char *) NULL;
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            resource_info.image_geometry=ConstantString(argv[i]);
            break;
          }
        if (LocaleCompare("profile",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            break;
          }
        ThrowDisplayException(OptionError,"UnrecognizedOption",option);
      }
      case 'q':
      {
        if (LocaleCompare("quality",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowDisplayInvalidArgumentException(option,argv[i]);
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
              ThrowDisplayException(OptionError,"MissingArgument",option);
            colorspace=ParseCommandOption(MagickColorspaceOptions,
              MagickFalse,argv[i]);
            if (colorspace < 0)
              ThrowDisplayException(OptionError,"UnrecognizedColorspace",
                argv[i]);
            break;
          }
        if (LocaleCompare("quiet",option+1) == 0)
          break;
        ThrowDisplayException(OptionError,"UnrecognizedOption",option);
      }
      case 'r':
      {
        if (LocaleCompare("raise",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowDisplayInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("regard-warnings",option+1) == 0)
          break;
        if (LocaleCompare("remote",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
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
              ThrowDisplayException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowDisplayInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("resample",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowDisplayInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("resize",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowDisplayInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleNCompare("respect-parentheses",option+1,17) == 0)
          {
            respect_parenthesis=(*option == '-') ? MagickTrue : MagickFalse;
            break;
          }
        if (LocaleCompare("roll",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowDisplayInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("rotate",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowDisplayInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowDisplayException(OptionError,"UnrecognizedOption",option);
      }
      case 's':
      {
        if (LocaleCompare("sample",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowDisplayInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("sampling-factor",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowDisplayInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("scenes",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            if (IsSceneGeometry(argv[i],MagickFalse) == MagickFalse)
              ThrowDisplayInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("seed",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowDisplayInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("segment",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowDisplayInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("set",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("sharpen",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowDisplayInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("shared-memory",option+1) == 0)
          {
            resource_info.use_shared_memory= (*option == '-') ? MagickTrue :
              MagickFalse;
            break;
          }
        if (LocaleCompare("size",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowDisplayInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("strip",option+1) == 0)
          break;
        if (LocaleCompare("support",option+1) == 0)
          {
            i++;  /* deprecated */
            break;
          }
        ThrowDisplayException(OptionError,"UnrecognizedOption",option);
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
              ThrowDisplayException(OptionError,"MissingArgument",option);
            resource_info.text_font=XGetResourceClass(resource_database,
              GetClientName(),"font",argv[i]);
            break;
          }
        if (LocaleCompare("texture",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("threshold",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowDisplayInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("thumbnail",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowDisplayInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("title",option+1) == 0)
          {
            resource_info.title=(char *) NULL;
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            resource_info.title=argv[i];
            break;
          }
        if (LocaleCompare("transparent-color",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("treedepth",option+1) == 0)
          {
            quantize_info->tree_depth=0;
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowDisplayInvalidArgumentException(option,argv[i]);
            quantize_info->tree_depth=StringToUnsignedLong(argv[i]);
            break;
          }
        if (LocaleCompare("trim",option+1) == 0)
          break;
        ThrowDisplayException(OptionError,"UnrecognizedOption",option);
      }
      case 'u':
      {
        if (LocaleCompare("update",option+1) == 0)
          {
            resource_info.update=(unsigned int) (*option == '-');
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowDisplayInvalidArgumentException(option,argv[i]);
            resource_info.update=(unsigned int) StringToUnsignedLong(argv[i]);
            break;
          }
        if (LocaleCompare("use-pixmap",option+1) == 0)
          {
            resource_info.use_pixmap=(*option == '-') ? MagickTrue :
              MagickFalse;
            break;
          }
        ThrowDisplayException(OptionError,"UnrecognizedOption",option);
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
        if (LocaleCompare("visual",option+1) == 0)
          {
            resource_info.visual_type=(char *) NULL;
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            resource_info.visual_type=argv[i];
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
              ThrowDisplayException(OptionError,"MissingArgument",option);
            method=ParseCommandOption(MagickVirtualPixelOptions,MagickFalse,
              argv[i]);
            if (method < 0)
              ThrowDisplayException(OptionError,
                "UnrecognizedVirtualPixelMethod",argv[i]);
            break;
          }
        ThrowDisplayException(OptionError,"UnrecognizedOption",option);
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
              ThrowDisplayException(OptionError,"MissingArgument",option);
            resource_info.window_id=argv[i];
            break;
          }
        if (LocaleCompare("window-group",option+1) == 0)
          {
            resource_info.window_group=(char *) NULL;
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            if (StringToDouble(argv[i],(char **) NULL) != 0)
              resource_info.window_group=argv[i];
            break;
          }
        if (LocaleCompare("write",option+1) == 0)
          {
            resource_info.write_filename=(char *) NULL;
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowDisplayException(OptionError,"MissingArgument",option);
            resource_info.write_filename=argv[i];
            if (IsPathAccessible(resource_info.write_filename) != MagickFalse)
              {
                char
                  answer[2],
                  *p;

                (void) FormatLocaleFile(stderr,"Overwrite %s? ",
                  resource_info.write_filename);
                p=fgets(answer,(int) sizeof(answer),stdin);
                (void) p;
                if (((*answer != 'y') && (*answer != 'Y')))
                  return(MagickFalse);
              }
            break;
          }
        ThrowDisplayException(OptionError,"UnrecognizedOption",option);
      }
      case '?':
        break;
      default:
        ThrowDisplayException(OptionError,"UnrecognizedOption",option);
    }
    fire=(GetCommandOptionFlags(MagickCommandOptions,MagickFalse,option) &
      FireOptionFlag) == 0 ?  MagickFalse : MagickTrue;
    if (fire != MagickFalse)
      FireImageStack(MagickFalse,MagickTrue,MagickTrue);
  }
  if (k != 0)
    ThrowDisplayException(OptionError,"UnbalancedParenthesis",argv[i]);
  if (state & RetainColorsState)
    {
      XRetainWindowColors(display,XRootWindow(display,XDefaultScreen(display)));
      (void) XSync(display,MagickFalse);
    }
  DestroyDisplay();
  return(status != 0 ? MagickTrue : MagickFalse);
#else
  wand_unreferenced(argc);
  wand_unreferenced(argv);
  wand_unreferenced(metadata);
  (void) ThrowMagickException(exception,GetMagickModule(),MissingDelegateError,
    "DelegateLibrarySupportNotBuiltIn","'%s' (X11)",image_info->filename);
  return(DisplayUsage());
#endif
}
