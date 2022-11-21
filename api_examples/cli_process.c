/*
   Direct call to ProcessCommandOptions() to process an array of
   options minus the command argument.  This is the function that
   actually splits up the argument array into separate operation
   group calls.


   Compile with ImageMagick-develop installed...

     gcc -lMagickWand -lMagickCore cli_process.c -o cli_process

   Compile and run directly from Source Directory...

     IM_PROG=api_examples/cli_process
     gcc -I`pwd` -LMagickWand/.libs -LMagickCore/.libs \
       -lMagickWand -lMagickCore  $IM_PROG.c -o $IM_PROG

     sh ./magick.sh    $IM_PROG

*/
#include <stdio.h>
#include "MagickCore/studio.h"
#include "MagickWand/MagickWand.h"

int main(int argc, char **argv)
{
  MagickCLI
    *cli_wand;

  int arg_count;
  char *args[] = { "-size", "100x100", "xc:red",
                   "(", "rose:", "-rotate", "-90", ")",
                   "+append", "show:", NULL };

  for(arg_count = 0; args[arg_count] != (char *) NULL; arg_count++);


  MagickCoreGenesis(argv[0],MagickFalse);
  cli_wand = AcquireMagickCLI((ImageInfo *) NULL,(ExceptionInfo *) NULL);

  ProcessCommandOptions(cli_wand, arg_count, args, 0);

  /* Note use of 'True' to report all exceptions - including non-fatals */
  if ( CLICatchException(cli_wand,MagickTrue) != MagickFalse )
    fprintf(stderr, "Major Error Detected\n");


  cli_wand = DestroyMagickCLI(cli_wand);
  MagickCoreTerminus();
}
