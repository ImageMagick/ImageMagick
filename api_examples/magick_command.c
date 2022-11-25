/*
   Direct call to MagickImageCommand(),
   which is basically what the "magick" command does via
   a wrapper function MagickCommandGenesis()

   Compile with ImageMagick-develop installed...

     gcc -lMagickWand -lMagickCore magick_command.c -o magick_command

   Compile and run directly from Source Directory...

     IM_PROG=api_examples/magick_command
     gcc -I`pwd` -LMagickWand/.libs -LMagickCore/.libs \
       -lMagickWand -lMagickCore  $IM_PROG.c -o $IM_PROG

     sh ./magick.sh $IM_PROG

*/
#include <stdio.h>
#include "MagickCore/studio.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickWand/MagickWand.h"
#include "MagickWand/magick-cli.h"

int main(int argc, char **argv)
{
  MagickCoreGenesis(argv[0],MagickFalse);

  {
    
    ImageInfo *image_info = AcquireImageInfo();
    ExceptionInfo *exception = AcquireExceptionInfo();

    int arg_count;
    char *args[] = { "magick", "-size", "100x100", "xc:red",
                     "(", "rose:", "-rotate", "-90", ")",
                     "+append", "show:", NULL };

    for(arg_count = 0; args[arg_count] != (char *) NULL; arg_count++);

    (void) MagickImageCommand(image_info, arg_count, args, NULL, exception);

    if (exception->severity != UndefinedException)
    {
      CatchException(exception);
      fprintf(stderr, "Major Error Detected\n");
    }

    image_info=DestroyImageInfo(image_info);
    exception=DestroyExceptionInfo(exception);
  }
  MagickCoreTerminus();
}
