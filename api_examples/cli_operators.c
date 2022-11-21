/*
   Make direct calls to process the various CLI option handling groups
   as defined in "MagickWand/operators.c" which uses a special
   MagickCLI type of 'wand'.

   This is essentially the calls 'ProcessCommandOptions()' make
   though without as many error and sanity checks.

   Compile with ImageMagick-develop installed...

     gcc -lMagickWand -lMagickCore cli_operators.c -o cli_operators

   Compile and run directly from Source Directory...

     IM_PROG=api_examples/cli_operators
     gcc -I`pwd` -LMagickWand/.libs -LMagickCore/.libs \
       -lMagickWand -lMagickCore  $IM_PROG.c -o $IM_PROG

     sh ./magick.sh    $IM_PROG


*/
#include <stdio.h>
#include "MagickCore/studio.h"
#include "MagickWand/MagickWand.h"
#include "MagickWand/operation.h"

int main(int argc, char **argv)
{
  MagickCLI
    *cli_wand;

  MagickCoreGenesis(argv[0],MagickFalse);

  cli_wand = AcquireMagickCLI((ImageInfo *) NULL,(ExceptionInfo *) NULL);

  CLIOption (cli_wand, "-size", "100x100");
  CLIOption (cli_wand, "-read", "xc:red");
  CLIOption (cli_wand, "(", NULL);
  CLIOption (cli_wand, "-read", "rose:");
  CLIOption (cli_wand, "-rotate", "-90", NULL);
  CLIOption (cli_wand, ")", NULL);
  CLIOption (cli_wand, "+append", NULL, NULL);
  CLIOption (cli_wand, "-write", "show:", NULL);

  /* Note use of 'True' to report all exceptions - including fatals */
  if ( CLICatchException(cli_wand,MagickTrue) != MagickFalse )
    fprintf(stderr, "Major Error Detected\n");

  cli_wand = DestroyMagickCLI(cli_wand);

  MagickCoreTerminus();
}
