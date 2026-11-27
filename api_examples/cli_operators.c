/*
   Make direct calls to process the various CLI option handling groups
   as defined in "MagickWand/operators.c" which uses a special
   MagickCLI type of 'wand'.

   This is essentially the calls 'ProcessCommandOptions()' make
   though without as many error and sanity checks.

   Compile with ImageMagick-develop installed...

     gcc cli_operators.c -o cli_operators \
       `pkg-config --cflags --libs MagickWand`

   (The source file must come BEFORE the libraries on the command line,
   otherwise linkers using --as-needed, the default on many distros,
   may drop the libraries and fail with undefined references.)

   Compile and run directly from Source Directory...

     IM_PROG=api_examples/cli_operators
     gcc -I`pwd` $IM_PROG.c -o $IM_PROG \
       -LMagickWand/.libs -LMagickCore/.libs -lMagickWand -lMagickCore

     sh ./magick.sh    $IM_PROG

   Note: the final "-write show:" needs a working display (X11 etc).
   Replace it with e.g. "-write", "result.png" to run headless.
*/
#include <stdio.h>
#include <stdlib.h>
#include "MagickWand/MagickWand.h"
#include "MagickWand/operation.h"

int main(int argc, char **argv)
{
  MagickCLI
    *cli_wand;

  int
    status;

  (void) argc;

  MagickCoreGenesis(argv[0],MagickFalse);

  cli_wand = AcquireMagickCLI((ImageInfo *) NULL,(ExceptionInfo *) NULL);
  if (cli_wand == (MagickCLI *) NULL)
    {
      fprintf(stderr, "Unable to acquire MagickCLI wand\n");
      MagickCoreTerminus();
      return(EXIT_FAILURE);
    }

  CLIOption (cli_wand, "-size",   "100x100", NULL);
  CLIOption (cli_wand, "-read",   "xc:red",  NULL);
  CLIOption (cli_wand, "(",       NULL,      NULL);
  CLIOption (cli_wand, "-read",   "rose:",   NULL);
  CLIOption (cli_wand, "-rotate", "-90",     NULL);
  CLIOption (cli_wand, ")",       NULL,      NULL);
  CLIOption (cli_wand, "+append", NULL,      NULL);
  CLIOption (cli_wand, "-write",  "show:",   NULL);

  /* Note use of MagickTrue to report all exceptions - including fatals */
  status = EXIT_SUCCESS;
  if ( CLICatchException(cli_wand,MagickTrue) != MagickFalse )
    {
      fprintf(stderr, "Major Error Detected\n");
      status = EXIT_FAILURE;
    }

  cli_wand = DestroyMagickCLI(cli_wand);

  MagickCoreTerminus();
  return(status);
}
