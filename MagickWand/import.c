/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                 IIIII  M   M  PPPP    OOO   RRRR   TTTTT                    %
%                   I    MM MM  P   P  O   O  R   R    T                      %
%                   I    M M M  PPPP   O   O  RRRR     T                      %
%                   I    M   M  P      O   O  R R      T                      %
%                 IIIII  M   M  P       OOO   R  R     T                      %
%                                                                             %
%                                                                             %
%                       Import Image from X11 Screen                          %
%                                                                             %
%                           Software Design                                   %
%                                Cristy                                       %
%                              July 1992                                      %
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
%  Use the import program to capture some or all of an X server screen and
%  save the image to a file.
%
*/

/*
  Include declarations.
*/
#include "MagickWand/studio.h"
#include "MagickWand/MagickWand.h"
#include "MagickWand/mogrify-private.h"
#include "MagickCore/string-private.h"
#include "MagickCore/image-private.h"
#include "MagickCore/xwindow-private.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   I m p o r t I m a g e C o m m a n d                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ImportImageCommand() reads an image from any visible window on an X server
%  and outputs it as an image file. You can capture a single window, the
%  entire screen, or any rectangular portion of the screen.  You can use the
%  display utility for redisplay, printing, editing, formatting, archiving,
%  image processing, etc. of the captured image.
%
%  The target window can be specified by id, name, or may be selected by
%  clicking the mouse in the desired window. If you press a button and then
%  drag, a rectangle will form which expands and contracts as the mouse moves.
%  To save the portion of the screen defined by the rectangle, just release
%  the button. The keyboard bell is rung once at the beginning of the screen
%  capture and twice when it completes.
%
%  The format of the ImportImageCommand method is:
%
%      MagickBooleanType ImportImageCommand(ImageInfo *image_info,int argc,
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

static MagickBooleanType ImportUsage(void)
{
  static const char
    miscellaneous[] =
      "  -debug events        display copious debugging information\n"
      "  -help                print program options\n"
      "  -list type           print a list of supported option arguments\n"
      "  -log format          format of debugging information\n"
      "  -version             print version information",
    operators[] =
      "  -annotate geometry text\n"
      "                       annotate the image with text\n"
      "  -colors value        preferred number of colors in the image\n"
      "  -crop geometry       preferred size and location of the cropped image\n"
      "  -encipher filename   convert plain pixels to cipher pixels\n"
      "  -extent geometry     set the image size\n"
      "  -geometry geometry   preferred size or location of the image\n"
      "  -help                print program options\n"
      "  -monochrome          transform image to black and white\n"
      "  -negate              replace every pixel with its complementary color \n"
      "  -quantize colorspace reduce colors in this colorspace\n"
      "  -resize geometry     resize the image\n"
      "  -rotate degrees      apply Paeth rotation to the image\n"
      "  -strip               strip image of all profiles and comments\n"
      "  -thumbnail geometry  create a thumbnail of the image\n"
      "  -transparent color   make this color transparent within the image\n"
      "  -trim                trim image edges\n"
      "  -type type           image type",
    settings[] =
      "  -adjoin              join images into a single multi-image file\n"
      "  -border              include window border in the output image\n"
      "  -channel type        apply option to select image channels\n"
      "  -colorspace type     alternate image colorspace\n"
      "  -comment string      annotate image with comment\n"
      "  -compress type       type of pixel compression when writing the image\n"
      "  -define format:option\n"
      "                       define one or more image format options\n"
      "  -density geometry    horizontal and vertical density of the image\n"
      "  -depth value         image depth\n"
      "  -descend             obtain image by descending window hierarchy\n"
      "  -display server      X server to contact\n"
      "  -dispose method      layer disposal method\n"
      "  -dither method       apply error diffusion to image\n"
      "  -delay value         display the next image after pausing\n"
      "  -encipher filename   convert plain pixels to cipher pixels\n"
      "  -endian type         endianness (MSB or LSB) of the image\n"
      "  -encoding type       text encoding type\n"
      "  -filter type         use this filter when resizing an image\n"
      "  -format \"string\"     output formatted image characteristics\n"
      "  -frame               include window manager frame\n"
      "  -gravity direction   which direction to gravitate towards\n"
      "  -identify            identify the format and characteristics of the image\n"
      "  -interlace type      None, Line, Plane, or Partition\n"
      "  -interpolate method  pixel color interpolation method\n"
      "  -label string        assign a label to an image\n"
      "  -limit type value    Area, Disk, Map, or Memory resource limit\n"
      "  -monitor             monitor progress\n"
      "  -page geometry       size and location of an image canvas\n"
      "  -pause seconds       seconds delay between snapshots\n"
      "  -pointsize value     font point size\n"
      "  -quality value       JPEG/MIFF/PNG compression level\n"
      "  -quiet               suppress all warning messages\n"
      "  -regard-warnings     pay attention to warning messages\n"
      "  -repage geometry     size and location of an image canvas\n"
      "  -respect-parentheses settings remain in effect until parenthesis boundary\n"
      "  -sampling-factor geometry\n"
      "                       horizontal and vertical sampling factor\n"
      "  -scene value         image scene number\n"
      "  -screen              select image from root window\n"
      "  -seed value          seed a new sequence of pseudo-random numbers\n"
      "  -set property value  set an image property\n"
      "  -silent              operate silently, i.e. don't ring any bells \n"
      "  -snaps value         number of screen snapshots\n"
      "  -support factor      resize support: > 1.0 is blurry, < 1.0 is sharp\n"
      "  -synchronize         synchronize image to storage device\n"
      "  -taint               declare the image as modified\n"
      "  -transparent-color color\n"
      "                       transparent color\n"
      "  -treedepth value     color tree depth\n"
      "  -verbose             print detailed information about the image\n"
      "  -virtual-pixel method\n"
      "                       Constant, Edge, Mirror, or Tile\n"
      "  -window id           select window with this id or name\n"
      "                       root selects whole screen";

  ListMagickVersion(stdout);
  (void) printf("Usage: %s [options ...] [ file ]\n",
    GetClientName());
  (void) printf("\nImage Settings:\n");
  (void) puts(settings);
  (void) printf("\nImage Operators:\n");
  (void) puts(operators);
  (void) printf("\nMiscellaneous Options:\n");
  (void) puts(miscellaneous);
  (void) printf(
  "\nBy default, 'file' is written in the MIFF image format.  To\n");
  (void) printf(
    "specify a particular image format, precede the filename with an image\n");
  (void) printf(
    "format name and a colon (i.e. ps:image) or specify the image type as\n");
  (void) printf(
    "the filename suffix (i.e. image.ps).  Specify 'file' as '-' for\n");
  (void) printf("standard input or output.\n");
  return(MagickTrue);
}

#if defined(MAGICKCORE_X11_DELEGATE)
WandExport MagickBooleanType ImportImageCommand(ImageInfo *image_info,
  int argc,char **argv,char **wand_unused(metadata),ExceptionInfo *exception)
{
#define DestroyImport() \
{ \
  XDestroyResourceInfo(&resource_info); \
  if (display != (Display *) NULL) \
    { \
      XCloseDisplay(display); \
      display=(Display *) NULL; \
    } \
  DestroyImageStack(); \
  if (target_window != (char *) NULL) \
    target_window=DestroyString(target_window); \
  for (i=0; i < (ssize_t) argc; i++) \
    argv[i]=DestroyString(argv[i]); \
  argv=(char **) RelinquishMagickMemory(argv); \
}
#define ThrowImportException(asperity,tag,option) \
{ \
  (void) ThrowMagickException(exception,GetMagickModule(),asperity,tag,"`%s'", \
     option); \
  DestroyImport(); \
  return(MagickFalse); \
}
#define ThrowImportInvalidArgumentException(option,argument) \
{ \
  (void) ThrowMagickException(exception,GetMagickModule(),OptionError, \
    "InvalidArgument","'%s': %s",option,argument); \
  DestroyImport(); \
  return(MagickFalse); \
}

  char
    *filename,
    *option,
    *resource_value,
    *server_name,
    *target_window;

  Display
    *display;

  Image
    *image;

  ImageStack
    image_stack[MaxImageStackDepth+1];

  MagickBooleanType
    fire,
    pend,
    respect_parentheses;

  MagickStatusType
    status;

  QuantizeInfo
    *quantize_info;

  ssize_t
    i;

  ssize_t
    j,
    k,
    snapshots;

  XImportInfo
    ximage_info;

  XResourceInfo
    resource_info;

  XrmDatabase
    resource_database;

  /*
    Set defaults.
  */
  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  wand_unreferenced(metadata);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
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
  display=(Display *) NULL;
  j=1;
  k=0;
  NewImageStack();
  option=(char *) NULL;
  pend=MagickFalse;
  resource_database=(XrmDatabase) NULL;
  respect_parentheses=MagickFalse;
  (void) memset(&resource_info,0,sizeof(resource_info));
  server_name=(char *) NULL;
  status=MagickTrue;
  SetNotifyHandlers;
  target_window=(char *) NULL;
  /*
    Check for server name specified on the command line.
  */
  ReadCommandlLine(argc,&argv);
  status=ExpandFilenames(&argc,&argv);
  if (status == MagickFalse)
    ThrowImportException(ResourceLimitError,"MemoryAllocationFailed",
      GetExceptionMessage(errno));
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
          ThrowImportException(OptionError,"MissingArgument",option);
        server_name=argv[i];
      }
    if ((LocaleCompare("help",option+1) == 0) ||
        (LocaleCompare("-help",option+1) == 0))
      {
        DestroyImport();
        return(ImportUsage());
      }
  }
  /*
    Get user defaults from X resource database.
  */
  display=XOpenDisplay(server_name);
  if (display == (Display *) NULL)
    ThrowImportException(XServerError,"UnableToOpenXServer",
      XDisplayName(server_name));
  (void) XSetErrorHandler(XError);
  resource_database=XGetResourceDatabase(display,GetClientName());
  XGetImportInfo(&ximage_info);
  XGetResourceInfo(image_info,resource_database,GetClientName(),
    &resource_info);
  quantize_info=resource_info.quantize_info;
  resource_value=XGetResourceInstance(resource_database,GetClientName(),
    "border","False");
  ximage_info.borders=IsStringTrue(resource_value);
  resource_value=XGetResourceInstance(resource_database,GetClientName(),
    "delay","0");
  resource_info.delay=(unsigned int) StringToUnsignedLong(resource_value);
  image_info->density=XGetResourceInstance(resource_database,GetClientName(),
    "density",(char *) NULL);
  resource_value=XGetResourceInstance(resource_database,GetClientName(),
    "descend","False");
  ximage_info.descend=IsStringTrue(resource_value);
  resource_value=XGetResourceInstance(resource_database,GetClientName(),
    "frame","False");
  ximage_info.frame=IsStringTrue(resource_value);
  resource_value=XGetResourceInstance(resource_database,GetClientName(),
    "interlace","none");
  image_info->interlace=UndefinedInterlace;
  if (LocaleCompare("None",resource_value) == 0)
    image_info->interlace=NoInterlace;
  if (LocaleCompare("Line",resource_value) == 0)
    image_info->interlace=LineInterlace;
  if (LocaleCompare("Plane",resource_value) == 0)
    image_info->interlace=PlaneInterlace;
  if (LocaleCompare("Partition",resource_value) == 0)
    image_info->interlace=PartitionInterlace;
  if (image_info->interlace == UndefinedInterlace)
    ThrowImportException(OptionError,"Unrecognized interlace type",
      resource_value);
  image_info->page=XGetResourceInstance(resource_database,GetClientName(),
    "pageGeometry",(char *) NULL);
  resource_value=XGetResourceInstance(resource_database,GetClientName(),
    "pause","0");
  resource_info.pause=(unsigned int) StringToUnsignedLong(resource_value);
  resource_value=XGetResourceInstance(resource_database,GetClientName(),
    "quality","85");
  image_info->quality=StringToUnsignedLong(resource_value);
  resource_value=XGetResourceInstance(resource_database,GetClientName(),
    "screen","False");
  ximage_info.screen=IsStringTrue(resource_value);
  resource_value=XGetResourceInstance(resource_database,GetClientName(),
    "silent","False");
  ximage_info.silent=IsStringTrue(resource_value);
  resource_value=XGetResourceInstance(resource_database,GetClientName(),
    "verbose","False");
  image_info->verbose=IsStringTrue(resource_value);
  resource_value=XGetResourceInstance(resource_database,GetClientName(),
    "dither","True");
  quantize_info->dither_method=IsStringTrue(resource_value) != MagickFalse ?
    RiemersmaDitherMethod : NoDitherMethod;
  snapshots=1;
  status=MagickTrue;
  filename=(char *) NULL;
  /*
    Check command syntax.
  */
  for (i=1; i < (ssize_t) argc; i++)
  {
    option=argv[i];
    if (LocaleCompare(option,"(") == 0)
      {
        FireImageStack(MagickFalse,MagickTrue,pend);
        if (k == MaxImageStackDepth)
          ThrowImportException(OptionError,"ParenthesisNestedTooDeeply",
            option);
        PushImageStack();
        continue;
      }
    if (LocaleCompare(option,")") == 0)
      {
        FireImageStack(MagickFalse,MagickTrue,MagickTrue);
        if (k == 0)
          ThrowImportException(OptionError,"UnableToParseExpression",option);
        PopImageStack();
        continue;
      }
    if (IsCommandOption(option) == MagickFalse)
      {
        Image
          *images;

        size_t
          scene;

        /*
          Read image from X server.
        */
        FireImageStack(MagickFalse,MagickFalse,pend);
        filename=argv[i];
        if (target_window != (char *) NULL)
          (void) CopyMagickString(image_info->filename,target_window,
            MagickPathExtent);
        for (scene=0; scene < (size_t) MagickMax(snapshots,1); scene++)
        {
          MagickDelay(1000*resource_info.pause);
          images=XImportImage(image_info,&ximage_info,exception);
          status&=(MagickStatusType) (images != (Image *) NULL) &&
            (exception->severity < ErrorException);
          if (images == (Image *) NULL)
            continue;
          (void) CopyMagickString(images->filename,filename,MagickPathExtent);
          (void) CopyMagickString(images->magick,"PS",MagickPathExtent);
          images->scene=scene;
          AppendImageStack(images);
        }
        continue;
      }
    pend=image != (Image *) NULL ? MagickTrue : MagickFalse;
    switch(*(option+1))
    {
      case 'a':
      {
        if (LocaleCompare("adjoin",option+1) == 0)
          break;
        if (LocaleCompare("annotate",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowImportException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowImportInvalidArgumentException(option,argv[i]);
            if (i == (ssize_t) argc)
              ThrowImportException(OptionError,"MissingArgument",option);
            i++;
            break;
          }
        ThrowImportException(OptionError,"UnrecognizedOption",option);
      }
      case 'b':
      {
        if (LocaleCompare("border",option+1) == 0)
          {
            (void) CopyMagickString(argv[i]+1,"sans",MagickPathExtent);
            ximage_info.borders=(*option == '-') ? MagickTrue : MagickFalse;
            break;
          }
        if (LocaleCompare("bordercolor",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowImportException(OptionError,"MissingArgument",option);
            break;
          }
        ThrowImportException(OptionError,"UnrecognizedOption",option);
      }
      case 'c':
      {
        if (LocaleCompare("cache",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowImportException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowImportInvalidArgumentException(option,argv[i]);
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
              ThrowImportException(OptionError,"MissingArgument",option);
            channel=ParseChannelOption(argv[i]);
            if (channel < 0)
              ThrowImportException(OptionError,"UnrecognizedChannelType",
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
              ThrowImportException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowImportInvalidArgumentException(option,argv[i]);
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
              ThrowImportException(OptionError,"MissingArgument",option);
            colorspace=ParseCommandOption(MagickColorspaceOptions,MagickFalse,
              argv[i]);
            if (colorspace < 0)
              ThrowImportException(OptionError,"UnrecognizedColorspace",
                argv[i]);
            break;
          }
        if (LocaleCompare("comment",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowImportException(OptionError,"MissingArgument",option);
            status=SetImageOption(image_info,"comment",argv[i]);
            if (status == MagickFalse)
              ThrowImportException(OptionError,"UnrecognizedOption",argv[i]);
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
              ThrowImportException(OptionError,"MissingArgument",option);
            compress=ParseCommandOption(MagickCompressOptions,MagickFalse,
              argv[i]);
            if (compress < 0)
              ThrowImportException(OptionError,"UnrecognizedImageCompression",
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
              ThrowImportException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowImportInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowImportException(OptionError,"UnrecognizedOption",option);
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
              ThrowImportException(OptionError,"MissingArgument",option);
            event=ParseCommandOption(MagickLogEventOptions,MagickFalse,argv[i]);
            if (event < 0)
              ThrowImportException(OptionError,"UnrecognizedEventType",argv[i]);
            (void) SetLogEventMask(argv[i]);
            break;
          }
        if (LocaleCompare("define",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowImportException(OptionError,"MissingArgument",option);
            if (*option == '+')
              {
                const char
                  *define;

                define=GetImageOption(image_info,argv[i]);
                if (define == (char *) NULL)
                  ThrowImportException(OptionError,"NoSuchOption",argv[i]);
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
              ThrowImportException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowImportInvalidArgumentException(option,argv[i]);
            status=SetImageOption(image_info,"delay",argv[i]);
            if (status == MagickFalse)
              ThrowImportException(OptionError,"UnrecognizedOption",argv[i]);
            break;
          }
        if (LocaleCompare("density",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowImportException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowImportInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("depth",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowImportException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowImportInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("descend",option+1) == 0)
          {
            ximage_info.descend=(*option == '-') ? MagickTrue : MagickFalse;
            break;
          }
        if (LocaleCompare("display",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowImportException(OptionError,"MissingArgument",option);
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
              ThrowImportException(OptionError,"MissingArgument",option);
            dispose=ParseCommandOption(MagickDisposeOptions,MagickFalse,argv[i]);
            if (dispose < 0)
              ThrowImportException(OptionError,"UnrecognizedDisposeMethod",
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
              ThrowImportException(OptionError,"MissingArgument",option);
            method=ParseCommandOption(MagickDitherOptions,MagickFalse,argv[i]);
            if (method < 0)
              ThrowImportException(OptionError,"UnrecognizedDitherMethod",
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
              ThrowImportException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowImportInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowImportException(OptionError,"UnrecognizedOption",option);
      }
      case 'e':
      {
        if (LocaleCompare("encipher",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowImportException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("encoding",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowImportException(OptionError,"MissingArgument",option);
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
              ThrowImportException(OptionError,"MissingArgument",option);
            endian=ParseCommandOption(MagickEndianOptions,MagickFalse,
              argv[i]);
            if (endian < 0)
              ThrowImportException(OptionError,"UnrecognizedEndianType",
                argv[i]);
            break;
          }
        if (LocaleCompare("extent",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowImportException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowImportInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowImportException(OptionError,"UnrecognizedOption",option);
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
              ThrowImportException(OptionError,"MissingArgument",option);
            filter=ParseCommandOption(MagickFilterOptions,MagickFalse,argv[i]);
            if (filter < 0)
              ThrowImportException(OptionError,"UnrecognizedImageFilter",
                argv[i]);
            break;
          }
        if (LocaleCompare("frame",option+1) == 0)
          {
            (void) CopyMagickString(argv[i]+1,"sans0",MagickPathExtent);
            ximage_info.frame=(*option == '-') ? MagickTrue : MagickFalse;
            break;
          }
        if (LocaleCompare("format",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowImportException(OptionError,"MissingArgument",option);
            break;
          }
        ThrowImportException(OptionError,"UnrecognizedOption",option);
      }
      case 'g':
      {
        if (LocaleCompare("geometry",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowImportException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowImportInvalidArgumentException(option,argv[i]);
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
              ThrowImportException(OptionError,"MissingArgument",option);
            gravity=ParseCommandOption(MagickGravityOptions,MagickFalse,argv[i]);
            if (gravity < 0)
              ThrowImportException(OptionError,"UnrecognizedGravityType",
                argv[i]);
            break;
          }
        ThrowImportException(OptionError,"UnrecognizedOption",option);
      }
      case 'h':
      {
        if (LocaleCompare("help",option+1) == 0)
          break;
        ThrowImportException(OptionError,"UnrecognizedOption",option);
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
              ThrowImportException(OptionError,"MissingArgument",option);
            interlace=ParseCommandOption(MagickInterlaceOptions,MagickFalse,
              argv[i]);
            if (interlace < 0)
              ThrowImportException(OptionError,"UnrecognizedInterlaceType",
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
              ThrowImportException(OptionError,"MissingArgument",option);
            interpolate=ParseCommandOption(MagickInterpolateOptions,MagickFalse,
              argv[i]);
            if (interpolate < 0)
              ThrowImportException(OptionError,"UnrecognizedInterpolateMethod",
                argv[i]);
            break;
          }
        ThrowImportException(OptionError,"UnrecognizedOption",option);
      }
      case 'l':
      {
        if (LocaleCompare("label",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowImportException(OptionError,"MissingArgument",option);
            status=SetImageOption(image_info,"label",argv[i]);
            if (status == MagickFalse)
              ThrowImportException(OptionError,"UnrecognizedOption",argv[i]);
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
              ThrowImportException(OptionError,"MissingArgument",option);
            resource=ParseCommandOption(MagickResourceOptions,MagickFalse,
              argv[i]);
            if (resource < 0)
              ThrowImportException(OptionError,"UnrecognizedResourceType",
                argv[i]);
            i++;
            if (i == (ssize_t) argc)
              ThrowImportException(OptionError,"MissingArgument",option);
            value=StringToDouble(argv[i],&p);
            (void) value;
            if ((p == argv[i]) && (LocaleCompare("unlimited",argv[i]) != 0))
              ThrowImportInvalidArgumentException(option,argv[i]);
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
              ThrowImportException(OptionError,"MissingArgument",option);
            list=ParseCommandOption(MagickListOptions,MagickFalse,argv[i]);
            if (list < 0)
              ThrowImportException(OptionError,"UnrecognizedListType",argv[i]);
            status=MogrifyImageInfo(image_info,(int) (i-j+1),(const char **)
              argv+j,exception);
            DestroyImport();
            return(status == 0 ? MagickFalse : MagickTrue);
          }
        if (LocaleCompare("log",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if ((i == (ssize_t) argc) || (strchr(argv[i],'%') == (char *) NULL))
              ThrowImportException(OptionError,"MissingArgument",option);
            break;
          }
        ThrowImportException(OptionError,"UnrecognizedOption",option);
      }
      case 'm':
      {
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
        ThrowImportException(OptionError,"UnrecognizedOption",option);
      }
      case 'n':
      {
        if (LocaleCompare("negate",option+1) == 0)
          break;
        ThrowImportException(OptionError,"UnrecognizedOption",option);
      }
      case 'p':
      {
        if (LocaleCompare("page",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowImportException(OptionError,"MissingArgument",option);
            status=SetImageOption(image_info,"page",argv[i]);
            if (status == MagickFalse)
              ThrowImportException(OptionError,"UnrecognizedOption",argv[i]);
            break;
          }
        if (LocaleCompare("pause",option+1) == 0)
          {
            resource_info.pause=0;
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowImportException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowImportInvalidArgumentException(option,argv[i]);
            resource_info.pause=(unsigned int) StringToUnsignedLong(argv[i]);
            break;
          }
        if (LocaleCompare("ping",option+1) == 0)
          break;  /* deprecated option */
        if (LocaleCompare("pointsize",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowImportException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowImportInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowImportException(OptionError,"UnrecognizedOption",option);
      }
      case 'q':
      {
        if (LocaleCompare("quality",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowImportException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowImportInvalidArgumentException(option,argv[i]);
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
              ThrowImportException(OptionError,"MissingArgument",option);
            colorspace=ParseCommandOption(MagickColorspaceOptions,
              MagickFalse,argv[i]);
            if (colorspace < 0)
              ThrowImportException(OptionError,"UnrecognizedColorspace",
                argv[i]);
            break;
          }
        if (LocaleCompare("quiet",option+1) == 0)
          break;
        ThrowImportException(OptionError,"UnrecognizedOption",option);
      }
      case 'r':
      {
        if (LocaleCompare("regard-warnings",option+1) == 0)
          break;
        if (LocaleCompare("repage",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowImportException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowImportInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("resize",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowImportException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowImportInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleNCompare("respect-parentheses",option+1,17) == 0)
          {
            respect_parentheses=(*option == '-') ? MagickTrue : MagickFalse;
            break;
          }
        if (LocaleCompare("rotate",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowImportException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowImportInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowImportException(OptionError,"UnrecognizedOption",option);
      }
      case 's':
      {
        if (LocaleCompare("sampling-factor",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowImportException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowImportInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("scene",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowImportException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowImportInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("set",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowImportException(OptionError,"MissingArgument",option);
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowImportException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("screen",option+1) == 0)
          {
            ximage_info.screen=(*option == '-') ? MagickTrue : MagickFalse;
            break;
          }
        if (LocaleCompare("seed",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowImportException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowImportInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("silent",option+1) == 0)
          {
            ximage_info.silent=(*option == '-') ? MagickTrue : MagickFalse;
            break;
          }
        if (LocaleCompare("snaps",option+1) == 0)
          {
            (void) CopyMagickString(argv[i]+1,"sans",MagickPathExtent);
            i++;
            if (i == (ssize_t) argc)
              ThrowImportException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowImportInvalidArgumentException(option,argv[i]);
            snapshots=(ssize_t) StringToLong(argv[i]);
            break;
          }
        if (LocaleCompare("strip",option+1) == 0)
          break;
        if (LocaleCompare("support",option+1) == 0)
          {
            i++;  /* deprecated */
            break;
          }
        if (LocaleCompare("synchronize",option+1) == 0)
          break;
        ThrowImportException(OptionError,"UnrecognizedOption",option);
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
              ThrowImportException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowImportInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("transparent",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowImportException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("transparent-color",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowImportException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("treedepth",option+1) == 0)
          {
            quantize_info->tree_depth=0;
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowImportException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowImportInvalidArgumentException(option,argv[i]);
            quantize_info->tree_depth=StringToUnsignedLong(argv[i]);
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
              ThrowImportException(OptionError,"MissingArgument",option);
            type=ParseCommandOption(MagickTypeOptions,MagickFalse,argv[i]);
            if (type < 0)
              ThrowImportException(OptionError,"UnrecognizedImageType",argv[i]);
            break;
          }
        ThrowImportException(OptionError,"UnrecognizedOption",option);
      }
      case 'w':
      {
        i++;
        if (i == (ssize_t) argc)
          ThrowImportException(OptionError,"MissingArgument",option);
        (void) CloneString(&target_window,argv[i]);
        break;
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
        ThrowImportException(OptionError,"UnrecognizedOption",option);
      }
      case '?':
        break;
      default:
        ThrowImportException(OptionError,"UnrecognizedOption",option);
    }
    fire=(GetCommandOptionFlags(MagickCommandOptions,MagickFalse,option) &
      FireOptionFlag) == 0 ?  MagickFalse : MagickTrue;
    if (fire != MagickFalse)
      FireImageStack(MagickFalse,MagickTrue,MagickTrue);
  }
  if (k != 0)
    ThrowImportException(OptionError,"UnbalancedParenthesis",argv[i]);
  if (i-- != (ssize_t) argc)
    ThrowImportException(OptionError,"MissingAnImageFilename",argv[i]);
  if (image == (Image *) NULL)
    ThrowImportException(OptionError,"MissingAnImageFilename",argv[argc-1]);
  FinalizeImageSettings(image_info,image,MagickTrue);
  status&=(MagickStatusType) WriteImages(image_info,image,filename,exception);
  DestroyImport();
  return(status != 0 ? MagickTrue : MagickFalse);
#else
WandExport MagickBooleanType ImportImageCommand(ImageInfo *image_info,
  int wand_unused(argc),char **wand_unused(argv),char **wand_unused(metadata),
  ExceptionInfo *exception)
{
  wand_unreferenced(argc);
  wand_unreferenced(argv);
  wand_unreferenced(metadata);
  (void) ThrowMagickException(exception,GetMagickModule(),MissingDelegateError,
    "DelegateLibrarySupportNotBuiltIn","'%s' (X11)",image_info->filename);
  return(ImportUsage());
#endif
}
