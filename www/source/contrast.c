#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <wand/MagickWand.h>

int main(int argc,char **argv)
{
#define QuantumScale  ((MagickRealType) 1.0/(MagickRealType) QuantumRange)
#define SigmoidalContrast(x) \
  (QuantumRange*(1.0/(1+exp(10.0*(0.5-QuantumScale*x)))-0.0066928509)*1.0092503)
#define ThrowWandException(wand) \
{ \
  char \
    *description; \
 \
  ExceptionType \
    severity; \
 \
  description=MagickGetException(wand,&severity); \
  (void) fprintf(stderr,"%s %s %lu %s\n",GetMagickModule(),description); \
  description=(char *) MagickRelinquishMemory(description); \
  exit(-1); \
}

  MagickBooleanType
    status;

  MagickPixelPacket
    pixel;

  MagickWand
    *contrast_wand,
    *image_wand;

  PixelIterator
    *contrast_iterator,
    *iterator;

  PixelWand
    **contrast_pixels,
    **pixels;

  register ssize_t
    x;

  size_t
    width;

  ssize_t
    y;

  if (argc != 3)
    {
      (void) fprintf(stdout,"Usage: %s image sigmoidal-image\n",argv[0]);
      exit(0);
    }
  /*
    Read an image.
  */
  MagickWandGenesis();
  image_wand=NewMagickWand();
  status=MagickReadImage(image_wand,argv[1]);
  if (status == MagickFalse)
    ThrowWandException(image_wand);
  contrast_wand=CloneMagickWand(image_wand);
  /*
    Sigmoidal non-linearity contrast control.
  */
  iterator=NewPixelIterator(image_wand);
  contrast_iterator=NewPixelIterator(contrast_wand);
  if ((iterator == (PixelIterator *) NULL) ||
      (contrast_iterator == (PixelIterator *) NULL))
    ThrowWandException(image_wand);
  for (y=0; y < (ssize_t) MagickGetImageHeight(image_wand); y++)
  {
    pixels=PixelGetNextIteratorRow(iterator,&width);
    contrast_pixels=PixelGetNextIteratorRow(contrast_iterator,&width);
    if ((pixels == (PixelWand **) NULL) ||
        (contrast_pixels == (PixelWand **) NULL))
      break;
    for (x=0; x < (ssize_t) width; x++)
    {
      PixelGetMagickColor(pixels[x],&pixel);
      pixel.red=SigmoidalContrast(pixel.red);
      pixel.green=SigmoidalContrast(pixel.green);
      pixel.blue=SigmoidalContrast(pixel.blue);
      pixel.index=SigmoidalContrast(pixel.index);
      PixelSetMagickColor(contrast_pixels[x],&pixel);
    }
    (void) PixelSyncIterator(contrast_iterator);
  }
  if (y < (ssize_t) MagickGetImageHeight(image_wand))
    ThrowWandException(image_wand);
  contrast_iterator=DestroyPixelIterator(contrast_iterator);
  iterator=DestroyPixelIterator(iterator);
  image_wand=DestroyMagickWand(image_wand);
  /*
    Write the image then destroy it.
  */
  status=MagickWriteImages(contrast_wand,argv[2],MagickTrue);
  if (status == MagickFalse)
    ThrowWandException(image_wand);
  contrast_wand=DestroyMagickWand(contrast_wand);
  MagickWandTerminus();
  return(0);
}
