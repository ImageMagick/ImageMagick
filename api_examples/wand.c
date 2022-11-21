/*
   Implementation of a CLI command using a MagickWand API

     magick -size 100x100 xc:red \
            \( rose: -rotate -90 \) \
            +append   show:


   Compile with ImageMagick-develop installed...

     gcc -lMagickWand -lMagickCore wand.c -o wand

   Compile and run directly from Source Directory...

     IM_PROG=api_examples/wand
     gcc -I`pwd` -LMagickWand/.libs -LMagickCore/.libs \
       -lMagickWand -lMagickCore  $IM_PROG.c -o $IM_PROG

     sh ./magick.sh    $IM_PROG

*/
#include <stdio.h>
#include "MagickWand/MagickWand.h"

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
      *red,     /* red image wand */
      *rose,    /* rose image wand */
      *output;  /* the appended output image */

    PixelWand
      *color;

    MagickBooleanType
      status;

    MagickWandGenesis();

    /* read in the red image */
    red = NewMagickWand();
    MagickSetSize(red,100,100);
    status = MagickReadImage(red, "xc:red" );
    if (status == MagickFalse)
      ThrowWandException(red);
    /* NOTE ABOUT MagickReadImage()
     * Unless the wand is empty set the first/last iterator to determine
     * if the read image(s) are to be prepend/append into that wand image
     * list.
     *
     * Setting a specific index always 'inserts' before that image.
     */

    /* read in the rose image */
    rose = NewMagickWand();
    status = MagickReadImage(rose, "rose:" );
    if (status == MagickFalse)
      ThrowWandException(rose);

    /* rotate the rose image - one image only */
    color=NewPixelWand();
    PixelSetColor(color, "white");
    status = MagickRotateImage(rose,color,-90.0);
    if (status == MagickFalse)
      ThrowWandException(rose);
    color = DestroyPixelWand(color);

    /* append rose image into the red image wand */
    MagickSetLastIterator(red);
    MagickAddImage(red,rose);
    rose = DestroyMagickWand(rose);  /* finished with 'rose' wand */
    /* NOTE ABOUT MagickAddImage()
     *
     * Always set the first/last image in the destination wand so that
     * IM knows if you want to prepend/append the images into that wands
     * image list.
     *
     * Setting a specific index always 'inserts' before that image.
     */

    /* append all images together to create the output wand */
    MagickSetFirstIterator(red);
    output = MagickAppendImages(red,MagickFalse);
    red = DestroyMagickWand(red);  /* finished with 'red' wand */
    /* NOTE ABOUT MagickAppendImages()
     *
     * It is important to either 'set first' or 'reset' the iterator before
     * appending images, as only images from current image onward are
     * appended together.
     *
     * Also note how a new wand is created by this operation, and that new
     * wand does not inherit any settings from the previous wand (at least not
     * at this time).
     */

    /* Final output */
    status = MagickWriteImage(output,"show:");
    if (status == MagickFalse)
      ThrowWandException(output);

    output = DestroyMagickWand(output);

    MagickWandTerminus();
}

/*
 * The above can be simplified further, though that is not what "magick"
 * command would do which we are simulating.
 *
 * Specifically you can read the 'rose' image directly on the end of of
 * 'red' image wand.  Then process just that rose image, even though it is
 * sharing the same wand as another image.
 *
 * Remember in MagickWand, simple image operators are only applied to the
 * current image in the wand an to no other image!  To apply a simple image
 * operator (like MagickRotateImage()) to all the images in a wand you must
 * iterate over all the images yourself.
 */

