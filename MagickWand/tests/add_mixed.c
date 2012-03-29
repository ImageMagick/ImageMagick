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
    *wand,    /* red image wand */
    *output;

  MagickBooleanType
    status;

  printf("Read 3 sets of 3 Images, each set with settings: none, first, last\n");
  printf("Result shoud be: 543 012 678\n");

  MagickWandGenesis();

  /* read in the red image */
  wand = NewMagickWand();

  /* add test from empty wand */
  status = MagickReadImage(wand, "font_0.gif" );
  if (status == MagickFalse)
    ThrowWandException(wand);

  status = MagickReadImage(wand, "font_1.gif" );
  if (status == MagickFalse)
    ThrowWandException(wand);

  status = MagickReadImage(wand, "font_2.gif" );
  if (status == MagickFalse)
    ThrowWandException(wand);

  /* add test to start */
  MagickSetFirstIterator(wand);
  status = MagickReadImage(wand, "font_3.gif" );
  if (status == MagickFalse)
    ThrowWandException(wand);

  status = MagickReadImage(wand, "font_4.gif" );
  if (status == MagickFalse)
    ThrowWandException(wand);

  status = MagickReadImage(wand, "font_5.gif" );
  if (status == MagickFalse)
    ThrowWandException(wand);

  /* add test to end */
  MagickSetLastIterator(wand);
  status = MagickReadImage(wand, "font_6.gif" );
  if (status == MagickFalse)
    ThrowWandException(wand);

  status = MagickReadImage(wand, "font_7.gif" );
  if (status == MagickFalse)
    ThrowWandException(wand);

  status = MagickReadImage(wand, "font_8.gif" );
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

