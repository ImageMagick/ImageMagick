/*
   Direct call to ProcessCommandOptions() to process an array of
   options minus the command argument.  This is the function that
   actually splits up the argument array into separate operation
   group calls.

   Compile with ImageMagick 7 development files installed...

     gcc cli_process_v2.c -o cli_process_v2 \
       $(pkg-config --cflags --libs MagickWand)

   Compile and run directly from Source Directory...

     IM_PROG=api_examples/cli_process_v2
     gcc -I`pwd` $IM_PROG.c -o $IM_PROG \
       -LMagickWand/.libs -LMagickCore/.libs \
       -lMagickWand -lMagickCore

     sh ./magick.sh    $IM_PROG

   Requires a display (or another delegate for "show:") to run
   successfully; on a headless machine replace "show:" with an output
   filename such as "out.png".
*/
#include <stdio.h>
#include <stdlib.h>
#include "MagickWand/MagickWand.h"

int main(int argc, char **argv)
{
  MagickCLI
    *cli_wand;

  int
    arg_count,
    status = EXIT_SUCCESS;

  char *args[] = { "-size", "100x100", "xc:red",
                   "(", "rose:", "-rotate", "-90", ")",
                   "+append", "show:" };

  (void) argc;
  arg_count = (int) (sizeof(args)/sizeof(args[0]));

  MagickCoreGenesis(argv[0],MagickFalse);

  cli_wand = AcquireMagickCLI((ImageInfo *) NULL,(ExceptionInfo *) NULL);
  if (cli_wand == (MagickCLI *) NULL)
    {
      fprintf(stderr,"Unable to allocate MagickCLI\n");
      MagickCoreTerminus();
      return(EXIT_FAILURE);
    }

  ProcessCommandOptions(cli_wand, arg_count, args, 0);

  /* Note use of 'True' to report all exceptions - including non-fatals */
  if ( CLICatchException(cli_wand,MagickTrue) != MagickFalse )
    {
      fprintf(stderr, "Major Error Detected\n");
      status = EXIT_FAILURE;
    }

  cli_wand = DestroyMagickCLI(cli_wand);
  MagickCoreTerminus();
  return(status);
}
