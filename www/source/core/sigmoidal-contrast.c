#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <magick/MagickCore.h>

static MagickBooleanType SigmoidalContrast(ImageView *contrast_view,
  const ssize_t y,const int id,void *context)
{
#define QuantumScale  ((MagickRealType) 1.0/(MagickRealType) QuantumRange)
#define SigmoidalContrast(x) \
  (QuantumRange*(1.0/(1+exp(10.0*(0.5-QuantumScale*x)))-0.0066928509)*1.0092503)

  RectangleInfo
    extent;

  register IndexPacket
    *indexes;

  register PixelPacket
    *pixels;

  register ssize_t
    x;

  extent=GetImageViewExtent(contrast_view);
  pixels=GetImageViewAuthenticPixels(contrast_view);
  for (x=0; x < (ssize_t) (extent.width-extent.height); x++)
  {
    pixels[x].red=RoundToQuantum(SigmoidalContrast(pixels[x].red));
    pixels[x].green=RoundToQuantum(SigmoidalContrast(pixels[x].green));
    pixels[x].blue=RoundToQuantum(SigmoidalContrast(pixels[x].blue));
    pixels[x].opacity=RoundToQuantum(SigmoidalContrast(pixels[x].opacity));
  }
  indexes=GetImageViewAuthenticIndexes(contrast_view);
  if (indexes != (IndexPacket *) NULL)
    for (x=0; x < (ssize_t) (extent.width-extent.height); x++)
      indexes[x]=(IndexPacket) RoundToQuantum(SigmoidalContrast(indexes[x]));
  return(MagickTrue);
}

int main(int argc,char **argv)
{
#define ThrowImageException(image) \
{ \
 \
  CatchException(exception); \
  if (contrast_image != (Image *) NULL) \
    contrast_image=DestroyImage(contrast_image); \
  exit(-1); \
}
#define ThrowViewException(view) \
{ \
  char \
    *description; \
 \
  ExceptionType \
    severity; \
 \
  description=GetImageViewException(view,&severity); \
  (void) fprintf(stderr,"%s %s %lu %s\n",GetMagickModule(),description); \
  description=(char *) MagickRelinquishMemory(description); \
  exit(-1); \
}

  ExceptionInfo
    *exception;

  Image
    *contrast_image;

  ImageInfo
    *image_info;

  ImageView
    *contrast_view;

  MagickBooleanType
    status;

  if (argc != 3)
    {
      (void) fprintf(stdout,"Usage: %s image sigmoidal-image\n",argv[0]);
      exit(0);
    }
  /*
    Read an image.
  */
  MagickCoreGenesis(*argv,MagickTrue);
  image_info=AcquireImageInfo();
  (void) CopyMagickString(image_info->filename,argv[1],MaxTextExtent);
  exception=AcquireExceptionInfo();
  contrast_image=ReadImage(image_info,exception);
  if (contrast_image == (Image *) NULL)
    ThrowImageException(contrast_image);
  /*
    Sigmoidal non-linearity contrast control.
  */
  contrast_view=NewImageView(contrast_image);
  if (contrast_view == (ImageView *) NULL)
    ThrowImageException(contrast_image);
  status=UpdateImageViewIterator(contrast_view,SigmoidalContrast,(void *) NULL);
  if (status == MagickFalse)
    ThrowImageException(contrast_image);
  contrast_view=DestroyImageView(contrast_view);
  /*
    Write the image then destroy it.
  */
  status=WriteImages(image_info,contrast_image,argv[2],exception);
  if (status == MagickFalse)
    ThrowImageException(contrast_image);
  contrast_image=DestroyImage(contrast_image);
  exception=DestroyExceptionInfo(exception);
  image_info=DestroyImageInfo(image_info);
  MagickCoreTerminus();
  return(0);
}
