#include <stdio.h>
#include <MagickWand/MagickWand.h>

/* set this to true to test loops methods with a empty wand */
#define TEST_EMPTY_WAND 0

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
    *output;

  MagickBooleanType
    status;

  MagickWandGenesis();

  printf("Read in a list of 6 images...\n");

  wand = NewMagickWand();
#if !TEST_EMPTY_WAND
  status = MagickReadImage(wand, "font_0.gif" )
        && MagickReadImage(wand, "font_1.gif" )
        && MagickReadImage(wand, "font_2.gif" )
        && MagickReadImage(wand, "font_3.gif" )
        && MagickReadImage(wand, "font_4.gif" )
        && MagickReadImage(wand, "font_5.gif" );
  if (status == MagickFalse)
    ThrowWandException(wand);
#endif

  printf("I actually read in %u images\n",
             (unsigned) MagickGetNumberImages(wand) );
  printf("\n");

  printf("After reading current image is #%d \"%s\"\n",
              (unsigned) MagickGetIteratorIndex(wand),
              MagickGetImageFilename(wand) );
  printf("\n");

  // Note that using MagickGetIteratorIndex() is slower than just
  // keeping track of the current image index yourself! But not a great cost.

  printf("Standard 'Reset while Next' loop through images\n");
  // keeping track of it to start with!
  MagickResetIterator(wand);
  while (MagickNextImage(wand) != MagickFalse)
    printf("image #%u \"%s\"\n",
              (unsigned) MagickGetIteratorIndex(wand),
              MagickGetImageFilename(wand) );
  printf("\n");

  printf("At this point, any image 'added' to wand will be appended!\n");
  printf("This special condition can be set by using either\n");
  printf("just         MagickSetLastIterator(w)\n");
  printf("or           MagickSetIteratorIndex(w,-1)\n");
  printf("\n");

  printf("Now that we are at the end, lets loop backward using 'Previous'\n");
  while (MagickPreviousImage(wand) != MagickFalse)
    printf("image #%u \"%s\"\n",
              (unsigned) MagickGetIteratorIndex(wand),
              MagickGetImageFilename(wand) );
  printf("\n");


  printf("Note at this point, any image 'added' to wand will be prepended!\n");
  printf("This special condition can be set by using either\n");
  printf("just         MagickSetFirstIterator(w)\n");
  printf("Or      MagickResetIterator(w); MagickPreviousImage(w);\n");
  printf("The latter method being the cause of the current condition\n");
  printf("\n");


  printf("Directly loop though images backward using 'Last, while Previous'\n");
  MagickSetLastIterator(wand);
  while ( MagickPreviousImage(wand) != MagickFalse )
    printf("image #%u \"%s\"\n",
              (unsigned) MagickGetIteratorIndex(wand),
              MagickGetImageFilename(wand) );
  printf("\n");


  printf("Loop through images using Indexes, in a weird flip-flop way!\n");
  printf("Note that indexing using a negative number, indexes from end \n");
  { ssize_t  i;
    ssize_t  n = (ssize_t) MagickGetNumberImages(wand);

    for ( i=0; i!=n;  i= (i>=0) ? -(i+1):-i ) {
      (void) MagickSetIteratorIndex(wand,i);
         /* Note that a return of MagickFalse by the above is not actually an
          * error (no exception will be generated).  It just means that the
          * index value used (positive or negative) is too large for the
          * size of the current image list  (EG: range error: -n <= i < n )
          * When it does happen, no change is made to the current image
          */
      printf("index %2d -> #%u \"%s\"\n", (int) i,
                (unsigned) MagickGetIteratorIndex(wand),
                MagickGetImageFilename(wand) );
    }
  }
  printf("\n");


  wand=DestroyMagickWand(wand);

  MagickWandTerminus();
}

