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

  printf("Just read images in three groups, no settings used\n");
  printf("Result should be: 012 345 678\n");

  MagickWandGenesis();

  wand = NewMagickWand();
  input = NewMagickWand();

  status = MagickReadImage(input, "font_0.gif" )
        && MagickReadImage(input, "font_1.gif" )
        && MagickReadImage(input, "font_2.gif" );
  if (status == MagickFalse)
    ThrowWandException(input);

  status = MagickAddImage(wand, input);
  if (status == MagickFalse)
    ThrowWandException(wand);

  ClearMagickWand(input);
  status = MagickReadImage(input, "font_3.gif" )
        && MagickReadImage(input, "font_4.gif" )
        && MagickReadImage(input, "font_5.gif" );
  if (status == MagickFalse)
    ThrowWandException(input);

  status = MagickAddImage(wand, input);
  if (status == MagickFalse)
    ThrowWandException(wand);
  ClearMagickWand(input);

  ClearMagickWand(input);
  status = MagickReadImage(input, "font_6.gif" )
        && MagickReadImage(input, "font_7.gif" )
        && MagickReadImage(input, "font_8.gif" );
  if (status == MagickFalse)
    ThrowWandException(input);

  status = MagickAddImage(wand, input);
  if (status == MagickFalse)
    ThrowWandException(wand);
  input=DestroyMagickWand(input);

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

