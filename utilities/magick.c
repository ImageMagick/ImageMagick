/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                 M   M   AAA    GGGG  IIIII   CCCC  K   K                    %
%                 MM MM  A   A  G        I    C      K  K                     %
%                 M M M  AAAAA  G GGG    I    C      KKK                      %
%                 M   M  A   A  G   G    I    C      K  K                     %
%                 M   M  A   A   GGGG  IIIII   CCCC  K   K                    %
%                                                                             %
%                                                                             %
%       Perform "Magick" on Images via the Command Line Interface             %
%                                                                             %
%                             Dragon Computing                                %
%                             Anthony Thyssen                                 %
%                               January 2012                                  %
%                                                                             %
%                                                                             %
%  Copyright 1999 ImageMagick Studio LLC, a non-profit organization           %
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
%  Read CLI arguments, script files, and pipelines, to provide options that
%  manipulate images from many different formats.
%
*/

/*
  Include declarations.
*/
#include "MagickWand/studio.h"
#include "MagickWand/MagickWand.h"
#include "MagickCore/resource-private.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%  M a i n                                                                    %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
*/

static int MagickMain(int argc,char **argv)
{
#define MagickCommandSize(name,use_metadata,command) \
  { (name), sizeof(name)-1, (use_metadata), (command) }

  typedef struct _CommandInfo
  {
    const char
      *client_name;

    size_t
      extent;

    MagickBooleanType
      use_metadata;

    MagickCommand
      command;
  } CommandInfo;

  const CommandInfo
    MagickCommands[] =
    {
      MagickCommandSize("magick", MagickFalse, MagickImageCommand),
#if !defined(MAGICKCORE_EXCLUDE_DEPRECATED)
      MagickCommandSize("convert", MagickFalse, ConvertImageCommand),
#endif
      MagickCommandSize("composite", MagickFalse, CompositeImageCommand),
      MagickCommandSize("identify", MagickTrue, IdentifyImageCommand),
      MagickCommandSize("animate", MagickFalse, AnimateImageCommand),
      MagickCommandSize("compare", MagickTrue, CompareImagesCommand),
      MagickCommandSize("conjure", MagickFalse, ConjureImageCommand),
      MagickCommandSize("display", MagickFalse, DisplayImageCommand),
      MagickCommandSize("import", MagickFalse, ImportImageCommand),
      MagickCommandSize("mogrify", MagickFalse, MogrifyImageCommand),
      MagickCommandSize("montage", MagickFalse, MontageImageCommand),
      MagickCommandSize("stream", MagickFalse, StreamImageCommand)
    };

  char
    client_name[MagickPathExtent],
    *metadata;

  ExceptionInfo
    *exception;

  ImageInfo
    *image_info;

  int
    exit_code,
    offset;

  MagickBooleanType
    status;

  size_t
    number_commands;

  ssize_t
    i;
  
#if defined(MAGICKCORE_EXCLUDE_DEPRECATED)
  if ((argc > 1) &&
      (LocaleNCompare("magick",argv[0],sizeof("magick")-1) == 0) &&
      (LocaleNCompare("convert",argv[1],sizeof("convert")-1) == 0))
    {
      (void) fprintf(stderr,"Use \"magick\" instead of the deprecated command \"magick convert\".\n");
      exit(1);
    }
#endif
  MagickCoreGenesis(*argv,MagickTrue);
  MagickWandGenesis();
  exception=AcquireExceptionInfo();
  image_info=AcquireImageInfo();
  GetPathComponent(argv[0],TailPath,client_name);
  number_commands=sizeof(MagickCommands)/sizeof(MagickCommands[0]);
  for (i=0; i < (ssize_t) number_commands; i++)
  {
    offset=LocaleNCompare(MagickCommands[i].client_name,client_name,
      MagickCommands[i].extent);
    if (offset == 0)
      break;
  }
  i%=(ssize_t) number_commands;
  if ((i == 0) && (argc > 1))
    {
      for (i=1; i < (ssize_t) number_commands; i++)
      {
        offset=LocaleCompare(MagickCommands[i].client_name,argv[1]);
        if (offset == 0)
          {
            argc--;
            argv++;
            break;
          }
      }
      i%=(ssize_t) number_commands;
    }
  metadata=(char *) NULL;
  status=MagickCommandGenesis(image_info,MagickCommands[i].command,argc,argv,
    MagickCommands[i].use_metadata ? &metadata : (char **) NULL,exception);
  if (metadata != (char *) NULL)
    {
      (void) fputs(metadata,stdout);
      metadata=DestroyString(metadata);
    }
  if (MagickCommands[i].command != CompareImagesCommand)
    exit_code=status != MagickFalse ? 0 : 1;
  else
    {
      if (status == MagickFalse)
        exit_code=2;
      else
        {
          const char
            *option;

          option=GetImageOption(image_info,"compare:dissimilar");
          exit_code=IsStringTrue(option) ? 1 : 0;
        }
    }
  image_info=DestroyImageInfo(image_info);
  exception=DestroyExceptionInfo(exception);
  MagickWandTerminus();
  return(exit_code);
}

#if !defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__)
int main(int argc,char **argv)
{
  return(MagickMain(argc,argv));
}
#else
static LONG WINAPI NTUncaughtException(EXCEPTION_POINTERS *info)
{
  magick_unreferenced(info);
  AsynchronousResourceComponentTerminus();
  return(EXCEPTION_CONTINUE_SEARCH);
}

int wmain(int argc,wchar_t *argv[])
{
  char
    **utf8;

  int
    i,
    status;

  SetUnhandledExceptionFilter(NTUncaughtException);
  SetConsoleOutputCP(CP_UTF8);
  utf8=NTArgvToUTF8(argc,argv);
  status=MagickMain(argc,utf8);
  for (i=0; i < argc; i++)
    utf8[i]=DestroyString(utf8[i]);
  utf8=(char **) RelinquishMagickMemory(utf8);
  return(status);
}
#endif
