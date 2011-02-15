#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <wand/MagickWand.h>

static MagickBooleanType SigmoidalContrast(WandView *contrast_view,
  const ssize_t y,const int id,void *context)
{
#define QuantumScale  ((MagickRealType) 1.0/(MagickRealType) QuantumRange)
#define SigmoidalContrast(x) \
  (QuantumRange*(1.0/(1+exp(10.0*(0.5-QuantumScale*x)))-0.0066928509)*1.0092503)

  RectangleInfo
    extent;

  MagickPixelPacket
    pixel;

  PixelWand
    **pixels;

  register ssize_t
    x;

  extent=GetWandViewExtent(contrast_view);
  pixels=GetWandViewPixels(contrast_view);
  for (x=0; x < (ssize_t) (extent.width-extent.x); x++)
  {
    PixelGetMagickColor(pixels[x],&pixel);
    pixel.red=SigmoidalContrast(pixel.red);
    pixel.green=SigmoidalContrast(pixel.green);
    pixel.blue=SigmoidalContrast(pixel.blue);
    pixel.index=SigmoidalContrast(pixel.index);
    PixelSetMagickColor(pixels[x],&pixel);
  }
  return(MagickTrue);
}

int main(int argc,char **argv)
{
#define ThrowViewException(view) \
{ \
  description=GetWandViewException(view,&severity); \
  (void) fprintf(stderr,"%s %s %lu %s\n",GetMagickModule(),description); \
  description=(char *) MagickRelinquishMemory(description); \
  exit(-1); \
}
#define ThrowWandException(wand) \
{ \
  description=MagickGetException(wand,&severity); \
  (void) fprintf(stderr,"%s %s %lu %s\n",GetMagickModule(),description); \
  description=(char *) MagickRelinquishMemory(description); \
  exit(-1); \
}

  char
    *description;

  ExceptionType
    severity;

  MagickBooleanType
    status;

  MagickPixelPacket
    pixel;

  MagickWand
    *contrast_wand;

  WandView
    *contrast_view;

  if (argc != 3)
    {
      (void) fprintf(stdout,"Usage: %s image sigmoidal-image\n",argv[0]);
      exit(0);
    }
  /*
    Read an image.
  */
  MagickWandGenesis();
  contrast_wand=NewMagickWand();
  status=MagickReadImage(contrast_wand,argv[1]);
  if (status == MagickFalse)
    ThrowWandException(contrast_wand);
  /*
    Sigmoidal non-linearity contrast control.
  */
  contrast_view=NewWandView(contrast_wand);
  if (contrast_view == (WandView *) NULL)
    ThrowWandException(contrast_wand);
  status=UpdateWandViewIterator(contrast_view,SigmoidalContrast,(void *) NULL);
  if (status == MagickFalse)
    ThrowWandException(contrast_wand);
  contrast_view=DestroyWandView(contrast_view);
  /*
    Write the image then destroy it.
  */
  status=MagickWriteImages(contrast_wand,argv[2],MagickTrue);
  if (status == MagickFalse)
    ThrowWandException(contrast_wand);
  contrast_wand=DestroyMagickWand(contrast_wand);
  MagickWandTerminus();
  return(0);
}
