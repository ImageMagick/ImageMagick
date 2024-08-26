/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                 SSSSS  TTTTT  RRRR   EEEEE   AAA   M   M                    %
%                 SS       T    R   R  E      A   A  MM MM                    %
%                  SSS     T    RRRR   EEE    AAAAA  M M M                    %
%                    SS    T    R R    E      A   A  M   M                    %
%                 SSSSS    T    R  R   EEEEE  A   A  M   M                    %
%                                                                             %
%                                                                             %
%                     Stream Image to a Raw Image Format                      %
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
%  Stream is a lightweight tool to stream one or more pixel components of the
%  image or portion of the image to your choice of storage formats. It writes
%  the pixel components as they are read from the input image a row at a time
%  making stream desirable when working with large images or when you require
%  raw pixel components.
%
*/

/*
  Include declarations.
*/
#include "MagickWand/studio.h"
#include "MagickWand/MagickWand.h"
#include "MagickWand/mogrify-private.h"
#include "MagickCore/stream-private.h"
#include "MagickCore/string-private.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   S t r e a m I m a g e C o m m a n d                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  StreamImageCommand() is a lightweight method designed to extract pixels
%  from large image files to a raw format using a minimum of system resources.
%  The entire image or any regular portion of the image can be extracted.
%
%  The format of the StreamImageCommand method is:
%
%      MagickBooleanType StreamImageCommand(ImageInfo *image_info,int argc,
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

static MagickBooleanType StreamUsage(void)
{
  static const char
    miscellaneous[] =
      "  -channel mask        set the image channel mask\n"
      "  -debug events        display copious debugging information\n"
      "  -help                print program options\n"
      "  -list type           print a list of supported option arguments\n"
      "  -log format          format of debugging information\n"
      "  -version             print version information",
    settings[] =
      "  -authenticate password\n"
      "                       decipher image with this password\n"
      "  -colorspace type     alternate image colorspace\n"
      "  -compress type       type of pixel compression when writing the image\n"
      "  -define format:option\n"
      "                       define one or more image format options\n"
      "  -density geometry    horizontal and vertical density of the image\n"
      "  -depth value         image depth\n"
      "  -extract geometry    extract area from image\n"
      "  -identify            identify the format and characteristics of the image\n"
      "  -interlace type      type of image interlacing scheme\n"
      "  -interpolate method  pixel color interpolation method\n"
      "  -limit type value    pixel cache resource limit\n"
      "  -map components      one or more pixel components\n"
      "  -monitor             monitor progress\n"
      "  -quantize colorspace reduce colors in this colorspace\n"
      "  -quiet               suppress all warning messages\n"
      "  -regard-warnings     pay attention to warning messages\n"
      "  -respect-parentheses settings remain in effect until parenthesis boundary\n"
      "  -sampling-factor geometry\n"
      "                       horizontal and vertical sampling factor\n"
      "  -seed value          seed a new sequence of pseudo-random numbers\n"
      "  -set attribute value set an image attribute\n"
      "  -size geometry       width and height of image\n"
      "  -storage-type type   pixel storage type\n"
      "  -synchronize         synchronize image to storage device\n"
      "  -taint               declare the image as modified\n"
      "  -transparent-color color\n"
      "                       transparent color\n"
      "  -verbose             print detailed information about the image\n"
      "  -virtual-pixel method\n"
      "                       virtual pixel access method";

  ListMagickVersion(stdout);
  (void) printf("Usage: %s [options ...] input-image raw-image\n",
    GetClientName());
  (void) printf("\nImage Settings:\n");
  (void) puts(settings);
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

WandExport MagickBooleanType StreamImageCommand(ImageInfo *image_info,
  int argc,char **argv,char **metadata,ExceptionInfo *exception)
{
#define DestroyStream() \
{ \
  DestroyImageStack(); \
  stream_info=DestroyStreamInfo(stream_info); \
  for (i=0; i < (ssize_t) argc; i++) \
    argv[i]=DestroyString(argv[i]); \
  argv=(char **) RelinquishMagickMemory(argv); \
}
#define ThrowStreamException(asperity,tag,option) \
{ \
  (void) ThrowMagickException(exception,GetMagickModule(),asperity,tag,"`%s'", \
    option); \
  DestroyStream(); \
  return(MagickFalse); \
}
#define ThrowStreamInvalidArgumentException(option,argument) \
{ \
  (void) ThrowMagickException(exception,GetMagickModule(),OptionError, \
    "InvalidArgument","'%s': %s",option,argument); \
  DestroyStream(); \
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

  StreamInfo
    *stream_info;

  /*
    Set defaults.
  */
  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  (void) metadata;
  if (argc == 2)
    {
      option=argv[1];
      if ((LocaleCompare("help",option+1) == 0) ||
          (LocaleCompare("-help",option+1) == 0))
        return(StreamUsage());
      if ((LocaleCompare("version",option+1) == 0) ||
          (LocaleCompare("-version",option+1) == 0))
        {
          ListMagickVersion(stdout);
          return(MagickFalse);
        }
    }
  if (argc < 3)
    {
      (void) ThrowMagickException(exception,GetMagickModule(),OptionError,
        "MissingArgument","%s","");
      (void) StreamUsage();
      return(MagickFalse);
    }
  format="%w,%h,%m";
  (void) format;
  j=1;
  k=0;
  NewImageStack();
  option=(char *) NULL;
  pend=MagickFalse;
  respect_parentheses=MagickFalse;
  stream_info=AcquireStreamInfo(image_info,exception);
  status=MagickTrue;
  /*
    Stream an image.
  */
  ReadCommandlLine(argc,&argv);
  status=ExpandFilenames(&argc,&argv);
  if (status == MagickFalse)
    ThrowStreamException(ResourceLimitError,"MemoryAllocationFailed",
      GetExceptionMessage(errno));
  status=OpenStream(image_info,stream_info,argv[argc-1],exception);
  if (status == MagickFalse)
    {
      DestroyStream();
      return(MagickFalse);
    }
  for (i=1; i < (ssize_t) (argc-1); i++)
  {
    option=argv[i];
    if (LocaleCompare(option,"(") == 0)
      {
        FireImageStack(MagickFalse,MagickTrue,pend);
        if (k == MaxImageStackDepth)
          ThrowStreamException(OptionError,"ParenthesisNestedTooDeeply",option);
        PushImageStack();
        continue;
      }
    if (LocaleCompare(option,")") == 0)
      {
        FireImageStack(MagickFalse,MagickTrue,MagickTrue);
        if (k == 0)
          ThrowStreamException(OptionError,"UnableToParseExpression",option);
        PopImageStack();
        continue;
      }
    if (IsCommandOption(option) == MagickFalse)
      {
        Image
          *images;

        /*
          Stream input image.
        */
        FireImageStack(MagickFalse,MagickFalse,pend);
        filename=argv[i];
        if ((LocaleCompare(filename,"--") == 0) && (i < (ssize_t) (argc-1)))
          filename=argv[++i];
        (void) CopyMagickString(image_info->filename,filename,MagickPathExtent);
        images=StreamImage(image_info,stream_info,exception);
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
        if (LocaleCompare("authenticate",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowStreamException(OptionError,"MissingArgument",option);
            break;
          }
        ThrowStreamException(OptionError,"UnrecognizedOption",option)
      }
      case 'c':
      {
        if (LocaleCompare("cache",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowStreamException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowStreamInvalidArgumentException(option,argv[i]);
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
              ThrowStreamException(OptionError,"MissingArgument",option);
            channel=ParseChannelOption(argv[i]);
            if (channel < 0)
              ThrowStreamException(OptionError,"UnrecognizedChannelType",
                argv[i]);
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
              ThrowStreamException(OptionError,"MissingArgument",option);
            colorspace=ParseCommandOption(MagickColorspaceOptions,MagickFalse,
              argv[i]);
            if (colorspace < 0)
              ThrowStreamException(OptionError,"UnrecognizedColorspace",
                argv[i]);
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
              ThrowStreamException(OptionError,"MissingArgument",option);
            compress=ParseCommandOption(MagickCompressOptions,MagickFalse,
              argv[i]);
            if (compress < 0)
              ThrowStreamException(OptionError,"UnrecognizedImageCompression",
                argv[i]);
            break;
          }
        if (LocaleCompare("concurrent",option+1) == 0)
          break;
        ThrowStreamException(OptionError,"UnrecognizedOption",option)
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
              ThrowStreamException(OptionError,"MissingArgument",option);
            event=ParseCommandOption(MagickLogEventOptions,MagickFalse,argv[i]);
            if (event < 0)
              ThrowStreamException(OptionError,"UnrecognizedEventType",argv[i]);
            (void) SetLogEventMask(argv[i]);
            break;
          }
        if (LocaleCompare("define",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowStreamException(OptionError,"MissingArgument",option);
            if (*option == '+')
              {
                const char
                  *define;

                define=GetImageOption(image_info,argv[i]);
                if (define == (const char *) NULL)
                  ThrowStreamException(OptionError,"NoSuchOption",argv[i]);
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
              ThrowStreamException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowStreamInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("depth",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowStreamException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowStreamInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("duration",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowStreamException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowStreamInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowStreamException(OptionError,"UnrecognizedOption",option)
      }
      case 'e':
      {
        if (LocaleCompare("extract",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowStreamException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowStreamInvalidArgumentException(option,argv[i]);
            break;
          }
        ThrowStreamException(OptionError,"UnrecognizedOption",option)
      }
      case 'h':
      {
        if ((LocaleCompare("help",option+1) == 0) ||
            (LocaleCompare("-help",option+1) == 0))
          {
            DestroyStream();
            return(StreamUsage());
          }
        ThrowStreamException(OptionError,"UnrecognizedOption",option)
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
              ThrowStreamException(OptionError,"MissingArgument",option);
            interlace=ParseCommandOption(MagickInterlaceOptions,MagickFalse,
              argv[i]);
            if (interlace < 0)
              ThrowStreamException(OptionError,"UnrecognizedInterlaceType",
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
              ThrowStreamException(OptionError,"MissingArgument",option);
            interpolate=ParseCommandOption(MagickInterpolateOptions,MagickFalse,
              argv[i]);
            if (interpolate < 0)
              ThrowStreamException(OptionError,"UnrecognizedInterpolateMethod",
                argv[i]);
            break;
          }
        ThrowStreamException(OptionError,"UnrecognizedOption",option)
      }
      case 'l':
      {
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
              ThrowStreamException(OptionError,"MissingArgument",option);
            resource=ParseCommandOption(MagickResourceOptions,MagickFalse,
              argv[i]);
            if (resource < 0)
              ThrowStreamException(OptionError,"UnrecognizedResourceType",
                argv[i]);
            i++;
            if (i == (ssize_t) argc)
              ThrowStreamException(OptionError,"MissingArgument",option);
            value=StringToDouble(argv[i],&p);
            (void) value;
            if ((p == argv[i]) && (LocaleCompare("unlimited",argv[i]) != 0))
              ThrowStreamInvalidArgumentException(option,argv[i]);
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
              ThrowStreamException(OptionError,"MissingArgument",option);
            list=ParseCommandOption(MagickListOptions,MagickFalse,argv[i]);
            if (list < 0)
              ThrowStreamException(OptionError,"UnrecognizedListType",argv[i]);
            status=MogrifyImageInfo(image_info,(int) (i-j+1),(const char **)
              argv+j,exception);
            DestroyStream();
            return(status == 0 ? MagickFalse : MagickTrue);
          }
        if (LocaleCompare("log",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if ((i == (ssize_t) argc) || (strchr(argv[i],'%') == (char *) NULL))
              ThrowStreamException(OptionError,"MissingArgument",option);
            break;
          }
        ThrowStreamException(OptionError,"UnrecognizedOption",option)
      }
      case 'm':
      {
        if (LocaleCompare("map",option+1) == 0)
          {
            (void) CopyMagickString(argv[i]+1,"san",MagickPathExtent);
            if (*option == '+')
              break;
            i++;
            SetStreamInfoMap(stream_info,argv[i]);
            break;
          }
        if (LocaleCompare("monitor",option+1) == 0)
          break;
        ThrowStreamException(OptionError,"UnrecognizedOption",option)
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
            if (i == (ssize_t) argc)
              ThrowStreamException(OptionError,"MissingArgument",option);
            colorspace=ParseCommandOption(MagickColorspaceOptions,
              MagickFalse,argv[i]);
            if (colorspace < 0)
              ThrowStreamException(OptionError,"UnrecognizedColorspace",
                argv[i]);
            break;
          }
        if (LocaleCompare("quiet",option+1) == 0)
          break;
        ThrowStreamException(OptionError,"UnrecognizedOption",option)
      }
      case 'r':
      {
        if (LocaleCompare("regard-warnings",option+1) == 0)
          break;
        if (LocaleNCompare("respect-parentheses",option+1,17) == 0)
          {
            respect_parentheses=(*option == '-') ? MagickTrue : MagickFalse;
            break;
          }
        ThrowStreamException(OptionError,"UnrecognizedOption",option)
      }
      case 's':
      {
        if (LocaleCompare("sampling-factor",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowStreamException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowStreamInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("seed",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowStreamException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowStreamInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("set",option+1) == 0)
          {
            i++;
            if (i == (ssize_t) argc)
              ThrowStreamException(OptionError,"MissingArgument",option);
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowStreamException(OptionError,"MissingArgument",option);
            break;
          }
        if (LocaleCompare("size",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowStreamException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowStreamInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("storage-type",option+1) == 0)
          {
            ssize_t
              type;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowStreamException(OptionError,"MissingArgument",option);
            type=ParseCommandOption(MagickStorageOptions,MagickFalse,argv[i]);
            if (type < 0)
              ThrowStreamException(OptionError,"UnrecognizedStorageType",
                argv[i]);
            SetStreamInfoStorageType(stream_info,(StorageType) type);
            break;
          }
        if (LocaleCompare("synchronize",option+1) == 0)
          break;
        ThrowStreamException(OptionError,"UnrecognizedOption",option)
      }
      case 't':
      {
        if (LocaleCompare("taint",option+1) == 0)
          break;
        if (LocaleCompare("transparent-color",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowStreamException(OptionError,"MissingArgument",option);
            break;
          }
        ThrowStreamException(OptionError,"UnrecognizedOption",option)
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
              ThrowStreamException(OptionError,"MissingArgument",option);
            method=ParseCommandOption(MagickVirtualPixelOptions,MagickFalse,
              argv[i]);
            if (method < 0)
              ThrowStreamException(OptionError,"UnrecognizedVirtualPixelMethod",
                argv[i]);
            break;
          }
        ThrowStreamException(OptionError,"UnrecognizedOption",option)
      }
      case '?':
        break;
      default:
        ThrowStreamException(OptionError,"UnrecognizedOption",option)
    }
    fire=(GetCommandOptionFlags(MagickCommandOptions,MagickFalse,option) &
      FireOptionFlag) == 0 ?  MagickFalse : MagickTrue;
    if (fire != MagickFalse)
      FireImageStack(MagickFalse,MagickTrue,MagickTrue);
  }
  if (k != 0)
    ThrowStreamException(OptionError,"UnbalancedParenthesis",argv[i]);
  if (i-- != (ssize_t) (argc-1))
    ThrowStreamException(OptionError,"MissingAnImageFilename",argv[i]);
  if (image == (Image *) NULL)
    ThrowStreamException(OptionError,"MissingAnImageFilename",argv[i]);
  FinalizeImageSettings(image_info,image,MagickTrue);
  if (image == (Image *) NULL)
    ThrowStreamException(OptionError,"MissingAnImageFilename",argv[i]);
  DestroyStream();
  return(status != 0 ? MagickTrue : MagickFalse);
}
