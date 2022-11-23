#include <stdio.h>
#include <MagickWand/MagickWand.h>

/* Simplify the exception handling
 * technically we should abort the program if
 *      severity >= ErrorException
 */
void ThrowWandException(MagickWand *wand)
{ char
  *description;

  ExceptionType
  severity;

  description=MagickGetException(wand,&severity);
  (void) fprintf(stderr,"%s %s %lu %s\n",GetMagickModule(),description);
  description=(char *) MagickRelinquishMemory(description);
}

/* useful function especially after appending two wands together */
#define SwapWands(a,b) { MagickWand *tmp=a; a=b; b=tmp; }

int main(int argc, char *argv[])
{
  MagickWand
    *wand,
    *input,
    *output;

  MagickBooleanType
    status;

  printf("Read 4 images, then set index 1 and read more individual images\n");
  printf("Result should be: 01 654 23\n");

  MagickWandGenesis();

  wand = NewMagickWand();
  input = NewMagickWand();

  status = MagickReadImage(input, "font_0.gif" )
        && MagickReadImage(input, "font_1.gif" )
        && MagickReadImage(input, "font_2.gif" )
        && MagickReadImage(input, "font_3.gif" );
  if (status == MagickFalse)
    ThrowWandException(input);

  status = MagickAddImage(wand, input);
  if (status == MagickFalse)
    ThrowWandException(wand);
  input=DestroyMagickWand(input);  /* finished */

  MagickSetIteratorIndex(wand, 1);

  status = MagickReadImage(wand, "font_4.gif" );
  if (status == MagickFalse)
    ThrowWandException(wand);

  status = MagickReadImage(wand, "font_5.gif" );
  if (status == MagickFalse)
    ThrowWandException(wand);

  status = MagickReadImage(wand, "font_6.gif" );
  if (status == MagickFalse)
    ThrowWandException(wand);

  /* append all images together to create the output wand */
  MagickResetIterator(wand); /* append all images */
  output = MagickAppendImages(wand,MagickFalse);
  wand = DestroyMagickWand(wand);  /* finished - could swap here */

  /* Final output */
  status = MagickWriteImage(output,"show:");
  if (status == MagickFalse)
    ThrowWandException(output);

  output = DestroyMagickWand(output);

  MagickWandTerminus();
}

