/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                CCCC   OOO   N   N  JJJJJ  U   U  RRRR   EEEEE               %
%               C      O   O  NN  N    J    U   U  R   R  E                   %
%               C      O   O  N N N    J    U   U  RRRR   EEE                 %
%               C      O   O  N  NN  J J    U   U  R R    E                   %
%                CCCC   OOO   N   N  JJJ     UUU   R  R   EEEEE               %
%                                                                             %
%                                                                             %
%                     Interpret Magick Scripting Language.                    %
%                                                                             %
%                              Software Design                                %
%                                   Cristy                                    %
%                               December 2001                                 %
%                                                                             %
%                                                                             %
%  Copyright 1999-2017 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://www.imagemagick.org/script/license.php                           %
%                                                                             %
%  Unless required by applicable law or agreed to in writing, software        %
%  distributed under the License is distributed on an "AS IS" BASIS,          %
%  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   %
%  See the License for the specific language governing permissions and        %
%  limitations under the License.                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  The conjure program gives you the ability to perform custom image processing
%  tasks from a script written in the Magick Scripting Language (MSL). MSL is
%  XML-based and consists of action statements with attributes. Actions include
%  reading an image, processing an image, getting attributes from an image,
%  writing an image, and more. An attribute is a key/value pair that modifies
%  the behavior of an action.
%
*/

/*
  Include declarations.
*/
#include "MagickWand/studio.h"
#include "MagickWand/MagickWand.h"
#include "MagickWand/mogrify-private.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   C o n j u r e I m a g e C o m m a n d                                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ConjureImageCommand() describes the format and characteristics of one or
%  more image files. It will also report if an image is incomplete or corrupt.
%  The information displayed includes the scene number, the file name, the
%  width and height of the image, whether the image is colormapped or not,
%  the number of colors in the image, the number of bytes in the image, the
%  format of the image (JPEG, PNM, etc.), and finally the number of seconds
%  it took to read and process the image.
%
%  The format of the ConjureImageCommand method is:
%
%      MagickBooleanType ConjureImageCommand(ImageInfo *image_info,int argc,
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

static MagickBooleanType ConjureUsage(void)
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
    *settings[]=
    {
      "-monitor             monitor progress",
      "-quiet               suppress all warning messages",
      "-regard-warnings     pay attention to warning messages",
      "-seed value          seed a new sequence of pseudo-random numbers",
      "-verbose             print detailed information about the image",
      (char *) NULL
    };

  ListMagickVersion(stdout);
  (void) printf("Usage: %s [options ...] file [ [options ...] file ...]\n",
    GetClientName());
  (void) printf("\nImage Settings:\n");
  for (p=settings; *p != (char *) NULL; p++)
    (void) printf("  %s\n",*p);
  (void) printf("\nMiscellaneous Options:\n");
  for (p=miscellaneous; *p != (char *) NULL; p++)
    (void) printf("  %s\n",*p);
  (void) printf("\nIn addition, define any key value pairs required by "
    "your script.  For\nexample,\n\n");
  (void) printf("    conjure -size 100x100 -color blue -foo bar script.msl\n");
  return(MagickFalse);
}

WandExport MagickBooleanType ConjureImageCommand(ImageInfo *image_info,
  int argc,char **argv,char **wand_unused(metadata),ExceptionInfo *exception)
{
#define DestroyConjure() \
{ \
  image=DestroyImageList(image); \
  for (i=0; i < (ssize_t) argc; i++) \
    argv[i]=DestroyString(argv[i]); \
  argv=(char **) RelinquishMagickMemory(argv); \
}
#define ThrowConjureException(asperity,tag,option) \
{ \
  (void) ThrowMagickException(exception,GetMagickModule(),asperity,tag,"`%s'", \
     option); \
  DestroyConjure(); \
  return(MagickFalse); \
}
#define ThrowConjureInvalidArgumentException(option,argument) \
{ \
  (void) ThrowMagickException(exception,GetMagickModule(),OptionError, \
    "InvalidArgument","'%s': %s",option,argument); \
  DestroyConjure(); \
  return(MagickFalse); \
}

  char
    filename[MagickPathExtent],
    *option;

  Image
    *image;

  MagickStatusType
    status;

  register ssize_t
    i;

  ssize_t
    number_images;

  wand_unreferenced(metadata);

  /*
    Set defaults.
  */
  assert(image_info != (ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"...");
  assert(exception != (ExceptionInfo *) NULL);
  if (argc < 2)
    return(ConjureUsage());
  image=NewImageList();
  number_images=0;
  option=(char *) NULL;
  /*
    Conjure an image.
  */
  ReadCommandlLine(argc,&argv);
  status=ExpandFilenames(&argc,&argv);
  if (status == MagickFalse)
    ThrowConjureException(ResourceLimitError,"MemoryAllocationFailed",
      GetExceptionMessage(errno));
  for (i=1; i < (ssize_t) argc; i++)
  {
    option=argv[i];
    if (IsCommandOption(option) != MagickFalse)
      {
        if (LocaleCompare("concurrent",option+1) == 0)
          break;
        if (LocaleCompare("debug",option+1) == 0)
          {
            ssize_t
              event;

            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConjureException(OptionError,"MissingArgument",option);
            event=ParseCommandOption(MagickLogEventOptions,MagickFalse,argv[i]);
            if (event < 0)
              ThrowConjureException(OptionError,"UnrecognizedEventType",
                argv[i]);
            (void) SetLogEventMask(argv[i]);
            continue;
          }
        if (LocaleCompare("duration",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConjureException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConjureInvalidArgumentException(option,argv[i]);
            continue;
          }
        if ((LocaleCompare("help",option+1) == 0) ||
            (LocaleCompare("-help",option+1) == 0))
          {
            if (*option == '-')
              return(ConjureUsage());
            continue;
          }
        if (LocaleCompare("log",option+1) == 0)
          {
            if (*option == '-')
              {
                i++;
                if (i == (ssize_t) argc)
                  ThrowConjureException(OptionError,"MissingLogFormat",option);
                (void) SetLogFormat(argv[i]);
              }
            continue;
          }
        if (LocaleCompare("monitor",option+1) == 0)
          continue;
        if (LocaleCompare("quiet",option+1) == 0)
          continue;
        if (LocaleCompare("regard-warnings",option+1) == 0)
          break;
        if (LocaleCompare("seed",option+1) == 0)
          {
            if (*option == '+')
              break;
            i++;
            if (i == (ssize_t) argc)
              ThrowConjureException(OptionError,"MissingArgument",option);
            if (IsGeometry(argv[i]) == MagickFalse)
              ThrowConjureInvalidArgumentException(option,argv[i]);
            break;
          }
        if (LocaleCompare("verbose",option+1) == 0)
          {
            image_info->verbose=(*option == '-') ? MagickTrue : MagickFalse;
            continue;
          }
        if ((LocaleCompare("version",option+1) == 0) ||
            (LocaleCompare("-version",option+1) == 0))
          {
            ListMagickVersion(stdout);
            return(MagickTrue);
          }
        /*
          Persist key/value pair.
        */
        (void) DeleteImageOption(image_info,option+1);
        status=SetImageOption(image_info,option+1,argv[i+1]);
        if (status == MagickFalse)
          ThrowConjureException(ImageError,"UnableToPersistKey",option);
        i++;
        continue;
      }
    /*
      Interpret MSL script.
    */
    (void) DeleteImageOption(image_info,"filename");
    status=SetImageOption(image_info,"filename",argv[i]);
    if (status == MagickFalse)
      ThrowConjureException(ImageError,"UnableToPersistKey",argv[i]);
    (void) FormatLocaleString(filename,MagickPathExtent,"msl:%s",argv[i]);
    image=ReadImages(image_info,filename,exception);
    CatchException(exception);
    if (image != (Image *) NULL)
      image=DestroyImageList(image);
    status=image != (Image *) NULL ? MagickTrue : MagickFalse;
    number_images++;
  }
  if (i != (ssize_t) argc)
    ThrowConjureException(OptionError,"MissingAnImageFilename",argv[i]);
  if (number_images == 0)
    ThrowConjureException(OptionError,"MissingAnImageFilename",argv[argc-1]);
  if (image != (Image *) NULL)
    image=DestroyImageList(image);
  for (i=0; i < (ssize_t) argc; i++)
    argv[i]=DestroyString(argv[i]);
  argv=(char **) RelinquishMagickMemory(argv);
  return(status != 0 ? MagickTrue : MagickFalse);
}
