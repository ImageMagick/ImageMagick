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
%                            CCCC  L      IIIII                               %
%                           C      L        I                                 %
%                           C      L        I                                 %
%                           C      L        I                                 %
%                            CCCC  LLLLL  IIIII                               %
%                                                                             %
%       Perform "Magick" on Images via the Command Line Interface             %
%                                                                             %
%                             Dragon Computing                                %
%                             Anthony Thyssen                                 %
%                               January 2012                                  %
%                                                                             %
%                                                                             %
%  Copyright 1999-2012 ImageMagick Studio LLC, a non-profit organization      %
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
%  Read CLI arguments, script files, and pipelines, to provide options that
%  manipulate images from many different formats.
%
*/

/*
  Include declarations.
*/
#include "MagickWand/studio.h"
#include "MagickWand/MagickWand.h"
#include "MagickWand/magick-wand-private.h"
#include "MagickWand/operation.h"
#include "MagickCore/version.h"
#include "MagickCore/string-private.h"
#include "MagickCore/utility-private.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   M a g i c k C o m m a n d S p e c i a l                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickS() Reads the various reads various options defining how
%  to read, process and write images. The options can be sourced from the
%  command line, or from script files, or pipelines.
%
%  Processing is performed using stack of expanded 'Wand' structures, which
%  not only define 'image_info' store of current global options, but also the
%  current input source of the options.
%
%  The format of the MagickImageCommand method is:
%
%      void MagickSpecialOption(MagickWand *wand,
%               const char *option, const char *arg)
%
%  A description of each parameter follows:
%
%    o wand: the main CLI Wand to use.
%
%    o option: The special option (with any switch char) to process
%
%    o arg: Argument for option, if required
%
*/
WandExport void MagickSpecialOption(MagickWand *wand,
     const char *option, const char *arg)
{
  if (LocaleCompare("-read",option) == 0)
    {
#if 1
      /* MagickCore style of Read */
      Image *
        new_images;

      CopyMagickString(wand->image_info->filename,arg,MaxTextExtent);
      if (wand->image_info->ping != MagickFalse)
        new_images=PingImages(wand->image_info,wand->exception);
      else
        new_images=ReadImages(wand->image_info,wand->exception);
      AppendImageToList(&wand->images, new_images);
#else
      /* MagickWand style of Read - append new images -- FAILS */
      MagickSetLastIterator(wand);
      MagickReadImage(wand, arg);
      MagickSetFirstIterator(wand);
#endif
      return;
    }
#if 0
  if (LocaleCompare(option,"(") == 0)
    // push images/settings
  if (LocaleCompare(option,")") == 0)
    // pop images/settings
  if (LocaleCompare(option,"respect_parenthesis") == 0)
    // adjust stack handling
  // Other 'special' options this should handle
  //    "region" "clone"  "list" "version" "noop" "sans*"?
  // It does not do "exit" however as due to its side-effect requirements
#endif
}
/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   M a g i c k C o m m a n d P r o c e s s O p t i o n s                     %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickCommandsProcessOptions() reads and processes arguments in the given
%  command line argument array.
%
%  The format of the MagickImageCommand method is:
%
%      void MagickCommandArgs(MagickWand *wand,int argc,char **argv)
%
%  A description of each parameter follows:
%
%    o wand: the main CLI Wand to use.
%
%    o argc: the number of elements in the argument vector.
%
%    o argv: A text array containing the command line arguments.
%
*/
#define MagickExceptionContinue(severity,tag,option,arg) \
  (void) ThrowMagickException(wand->exception,GetMagickModule(),severity,tag, \
       "'%s' arg#%lu", option, (unsigned long)arg)
#define MagickExceptionReturn(severity,tag,option,arg) \
{ \
  MagickExceptionContinue(severity,tag,option,arg); \
  return; \
}

WandExport void MagickCommandProcessOptions(MagickWand *wand,int argc,
     char **argv)
{
  const char
    *option,
    *arg1,
    *arg2;

  MagickBooleanType
    plus_alt_op;

  ssize_t
    i,
    count;

  CommandOptionFlags
    flags;

  assert(wand != (MagickWand *) NULL);
  assert(wand->signature == WandSignature);
  assert(wand->draw_info != (DrawInfo *) NULL); /* ensure it is a CLI wand */
  if (wand->debug != MagickFalse)
    (void) LogMagickEvent(WandEvent,GetMagickModule(),"%s",wand->name);

  /*
    Parse command-line options.
  */
  count=0;
  for (i=1; i < (ssize_t) (argc-1); i+=count+1)
    {
      option=argv[i];
      plus_alt_op = MagickFalse;
      arg1=(char *)NULL;
      arg2=(char *)NULL;

      /* FUTURE: merge these into one call */
      count=ParseCommandOption(MagickCommandOptions,MagickFalse,argv[i]);
      flags=(CommandOptionFlags) GetCommandOptionFlags(
                   MagickCommandOptions,MagickFalse,argv[i]);

#define MagickCommandDebug 0

      if ( count == -1 || flags == UndefinedOptionFlag ||
           (flags & NonConvertOptionFlag) != 0 )
        {
          count = 0;
#if MagickCommandDebug
      (void) FormatLocaleFile(stderr, "CLI Non-Option: \"%s\"\n", option);
#endif
          if (IsCommandOption(option) == MagickFalse)
            {
              /* non-option -- treat as a image read */
              MagickSpecialOption(wand,"-read",option);
              continue;
            }
          else
            MagickExceptionReturn(OptionError,"UnrecognizedOption",option,i);
        }

      if ( (flags & DeprecateOptionFlag) != 0 )
        MagickExceptionContinue(OptionWarning,"DeprecatedOption",option,i);
        /* continue processing option anyway */

      if ((i+count) > (ssize_t) (argc-1))
        MagickExceptionReturn(OptionError,"MissingArgument",option,i);
      if (*option=='+') plus_alt_op = MagickTrue;
      if (*option!='+') arg1 = "true";
      if ( count >= 1 ) arg1 = argv[i+1];
      if ( count >= 2 ) arg2 = argv[i+2];

#if MagickCommandDebug
      (void) FormatLocaleFile(stderr,
          "CLI Option: \"%s\" \tCount: %d  Flags: %04x Args: \"%s\" \"%s\"\n",
          option,count,flags,arg1,arg2);
#endif

      if ( (flags & SpecialOptionFlag) != 0 )
        {
          if (LocaleCompare(option,"-exit") == 0)
            return;
#if 0
          if (LocaleCompare(option,"-script") == 0)
            {
              // Unbalanced Parenthesis if stack not empty
              // Call Script, with filename as argv[0]
              return;
            }
#endif
          MagickSpecialOption(wand,option,arg1);
        }

      if ( (flags & SettingOptionFlags) != 0 )
        {
          WandSettingOptionInfo(wand, option+1, arg1);
          // FUTURE: Sync Specific Settings into Images
        }

      if ( (flags & SimpleOperatorOptionFlag) != 0)
        {
          WandSimpleOperatorImages(wand, plus_alt_op, option+1, arg1, arg2);
        }

      if ( (flags & ListOperatorOptionFlag) != 0 )
        {
          WandListOperatorImages(wand, plus_alt_op, option+1, arg1, arg2);
        }

      // FUTURE: '-regard_warning' causes IM to exit more prematurely!
      // Note pipelined options may like more control over this level
      if (wand->exception->severity > ErrorException)
        {
          if (wand->exception->severity > ErrorException)
              //(regard_warnings != MagickFalse))
            return;                        /* FATAL - let caller handle */
          CatchException(wand->exception); /* output warnings and clear!!! */
        }
    }

  /* FUTURE: in the following produce a better error report
     -- Missing Output filename
  */

  assert(i!=(ssize_t)(argc-1));
  option=argv[i];  /* the last argument - output filename! */

#if MagickCommandDebug
  (void) FormatLocaleFile(stderr, "CLI Output: \"%s\"\n", option );
#endif

  // if stacks are not empty
  //  ThrowConvertException(OptionError,"UnbalancedParenthesis",option,i);


  /* This is the only value 'do no write' option for a CLI */
  if (LocaleCompare(option,"-exit") == 0 )
    return;  /* just exit, no image write */

  /* If there is an option -- produce an error */
  if (IsCommandOption(option) != MagickFalse)
    MagickExceptionReturn(OptionError,"MissingAnImageFilename",option,i);

  /* If no images */
  if ( wand->images == (Image *) NULL )
    {
      /* a "null:" output coder with no images is ok */
      if ( LocaleCompare(option,"null:") == 0 )
        return;
      MagickExceptionReturn(OptionError,"MissingAnImageFilename",option,i);
    }

  /*
     Write out final image!
  */
  //WandListOperatorImages(wand,MagickFalse,"write",option,(const char *)NULL);
  (void) SyncImagesSettings(wand->image_info,wand->images,wand->exception);
  (void) WriteImages(wand->image_info,wand->images,option,wand->exception);

  return;
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   M a g i c k I m a g e C o m m a n d                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  MagickImageCommand() Handle special 'once only' CLI arguments and
%  prepare to process the command line using a special CLI Magick Wand
%  via the MagickImageProcess() function.
%
%  The format of the MagickImageCommand method is:
%
%      MagickBooleanType MagickImageCommand(ImageInfo *image_info,
%           int argc, char **argv, char **metadata, ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the starting image_info structure
%         (for compatibilty with MagickCommandGenisis())
%
%    o argc: the number of elements in the argument vector.
%
%    o argv: A text array containing the command line arguments.
%
%    o metadata: any metadata is returned here.
%         (for compatibilty with MagickCommandGenisis())
%
%    o exception: return any errors or warnings in this structure.
%
*/

static MagickBooleanType MagickUsage(void)
{
  printf("Version: %s\n",GetMagickVersion((size_t *) NULL));
  printf("Copyright: %s\n",GetMagickCopyright());
  printf("Features: %s\n\n",GetMagickFeatures());
  printf("\n");

  printf("Usage: %s [(options|images) ...] output_image\n", GetClientName());
  printf("       %s -script filename [script args...]\n", GetClientName());
  printf("       ... | %s -script - | ...\n", GetClientName());
  printf("\n");

  printf("  For more information on usage, options, examples, and technqiues\n");
  printf("  see the ImageMagick website at\n    %s\n", MagickAuthoritativeURL);
  printf("  Or the web pages in ImageMagick Sources\n");
  return(MagickFalse);
}

/*
   Concatanate given file arguments to the given output argument.
   Used for a special -concatenate option used for specific 'delegates'.
   The option is not formally documented.

      magick -concatenate files... output

   This is much like the UNIX "cat" command, but for both UNIX and Windows,
   however the last argument provides the output filename.
*/
#define ThrowFileException(exception,severity,tag,context) \
{ \
  char \
    *message; \
 \
  message=GetExceptionMessage(errno); \
  (void) ThrowMagickException(exception,GetMagickModule(),severity, \
    tag == (const char *) NULL ? "unknown" : tag,"`%s': %s",context,message); \
  message=DestroyString(message); \
}

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
      ThrowFileException(exception,FileOpenError,"UnableToOpenFile",argv[i]);
    for (c=fgetc(input); c != EOF; c=fgetc(input))
      (void) fputc((char) c,output);
    (void) fclose(input);
    (void) remove_utf8(argv[i]);
  }
  (void) fclose(output);
  return(MagickTrue);
}

WandExport MagickBooleanType MagickImageCommand(ImageInfo *image_info,
  int argc,char **argv,char **metadata,ExceptionInfo *exception)
{
  MagickWand
    *wand;

  const char
    *option;

  /* Handle special single use options */
  if (argc == 2)
    {
      option=argv[1];
      if ((LocaleCompare("-version",option+1) == 0) ||
          (LocaleCompare("--version",option+1) == 0) )
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
    return(MagickUsage());
  ReadCommandlLine(argc,&argv);
#if 0
  status=ExpandFilenames(&argc,&argv);
  if (status == MagickFalse)
    ThrowConvertException(ResourceLimitError,"MemoryAllocationFailed",
      GetExceptionMessage(errno));
#endif
  if (LocaleCompare("-concatenate",argv[1]) == 0)
    return(ConcatenateImages(argc,argv,exception));

  /* create a special CLI Wand to hold all working settings */
  /* FUTURE: add this to 'operations.c' */
  wand=NewMagickWand();
  wand->image_info=DestroyImageInfo(wand->image_info);
  wand->image_info=image_info;
  wand->exception=DestroyExceptionInfo(wand->exception);
  wand->exception=exception;
  wand->draw_info=CloneDrawInfo(image_info,(DrawInfo *) NULL);
  wand->quantize_info=AcquireQuantizeInfo(image_info);

  if (LocaleCompare("-list",argv[1]) == 0)
    WandSettingOptionInfo(wand, argv[1]+1, argv[2]);
  else
    MagickCommandProcessOptions(wand,argc,argv);

  assert(wand->exception == exception);
  assert(wand->image_info == image_info);

  /* Handle metadata for ImageMagickObject COM object for Windows VBS */
  if (metadata != (char **) NULL)
    {
      const char
        *format;

      char
        *text;

      format="%w,%h,%m";   // Get this from image_info Option splaytree

      text=InterpretImageProperties(image_info,wand->images,format,exception);
      if (text == (char *) NULL)
        ThrowMagickException(exception,GetMagickModule(),ResourceLimitError,
             "MemoryAllocationFailed","`%s'", GetExceptionMessage(errno));
      else
        {
          (void) ConcatenateString(&(*metadata),text);
          text=DestroyString(text);
        }
    }

  /* Destroy the special CLI Wand */
  wand->exception = (ExceptionInfo *)NULL;
  wand->image_info = (ImageInfo *)NULL;
  wand=DestroyMagickWand(wand);

  return((exception->severity > ErrorException) ? MagickFalse : MagickTrue);
}
