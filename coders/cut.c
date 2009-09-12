/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                              CCC  U   U  TTTTT                              %
%                             C     U   U    T                                %
%                             C     U   U    T                                %
%                             C     U   U    T                                %
%                              CCC   UUU     T                                %
%                                                                             %
%                                                                             %
%                         Read DR Halo Image Format                           %
%                                                                             %
%                              Software Design                                %
%                              Jaroslav Fojtik                                %
%                                 June 2000                                   %
%                                                                             %
%                                                                             %
%  Permission is hereby granted, free of charge, to any person obtaining a    %
%  copy of this software and associated documentation files ("ImageMagick"),  %
%  to deal in ImageMagick without restriction, including without limitation   %
%  the rights to use, copy, modify, merge, publish, distribute, sublicense,   %
%  and/or sell copies of ImageMagick, and to permit persons to whom the       %
%  ImageMagick is furnished to do so, subject to the following conditions:    %
%                                                                             %
%  The above copyright notice and this permission notice shall be included in %
%  all copies or substantial portions of ImageMagick.                         %
%                                                                             %
%  The software is provided "as is", without warranty of any kind, express or %
%  implied, including but not limited to the warranties of merchantability,   %
%  fitness for a particular purpose and noninfringement.  In no event shall   %
%  ImageMagick Studio be liable for any claim, damages or other liability,    %
%  whether in an action of contract, tort or otherwise, arising from, out of  %
%  or in connection with ImageMagick or the use or other dealings in          %
%  ImageMagick.                                                               %
%                                                                             %
%  Except as contained in this notice, the name of the ImageMagick Studio     %
%  shall not be used in advertising or otherwise to promote the sale, use or  %
%  other dealings in ImageMagick without prior written authorization from the %
%  ImageMagick Studio.                                                        %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%
*/

/*
  Include declarations.
*/
#include "magick/studio.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/cache.h"
#include "magick/color.h"
#include "magick/color-private.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"
#include "magick/utility.h"

typedef struct
{
  unsigned Width;
  unsigned Height;
  unsigned Reserved;
} CUTHeader;

typedef struct
{
  char FileId[2];
  unsigned Version;
  unsigned Size;
  char FileType;
  char SubType;
  unsigned BoardID;
  unsigned GraphicsMode;
  unsigned MaxIndex;
  unsigned MaxRed;
  unsigned MaxGreen;
  unsigned MaxBlue;
  char PaletteId[20];
} CUTPalHeader;


static void InsertRow(long depth,unsigned char *p,long y,Image *image)
{
  ExceptionInfo
    *exception;

  unsigned long bit; long x;
  register PixelPacket *q;
  IndexPacket index;
  register IndexPacket *indexes;


  index=(IndexPacket) 0;
  exception=(&image->exception);
  switch (depth)
    {
    case 1:  /* Convert bitmap scanline. */
      {
        q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        indexes=GetAuthenticIndexQueue(image);
        for (x=0; x < ((long) image->columns-7); x+=8)
          {
            for (bit=0; bit < 8; bit++)
              {
                index=(IndexPacket) ((((*p) & (0x80 >> bit)) != 0) ? 0x01 : 0x00);
                indexes[x+bit]=index;
                *q++=image->colormap[(long) index];
              }
            p++;
          }
        if ((image->columns % 8) != 0)
          {
            for (bit=0; bit < (image->columns % 8); bit++)
              {
                index=(IndexPacket) ((((*p) & (0x80 >> bit)) != 0) ? 0x01 : 0x00);
                indexes[x+bit]=index;
                *q++=image->colormap[(long) index];
              }
            p++;
          }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
        /*            if (image->previous == (Image *) NULL)
                      if (QuantumTick(y,image->rows) != MagickFalse)
                      ProgressMonitor(LoadImageText,image->rows-y-1,image->rows);*/
        break;
      }
    case 2:  /* Convert PseudoColor scanline. */
      {
        q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        indexes=GetAuthenticIndexQueue(image);
        for (x=0; x < ((long) image->columns-1); x+=2)
          {
            index=ConstrainColormapIndex(image,(*p >> 6) & 0x3);
            indexes[x]=index;
            *q++=image->colormap[(long) index];
            index=ConstrainColormapIndex(image,(*p >> 4) & 0x3);
            indexes[x]=index;
            *q++=image->colormap[(long) index];
            index=ConstrainColormapIndex(image,(*p >> 2) & 0x3);
            indexes[x]=index;
            *q++=image->colormap[(long) index];
            index=ConstrainColormapIndex(image,(*p) & 0x3);
            indexes[x+1]=index;
            *q++=image->colormap[(long) index];
            p++;
          }
        if ((image->columns % 4) != 0)
          {
            index=ConstrainColormapIndex(image,(*p >> 6) & 0x3);
            indexes[x]=index;
            *q++=image->colormap[(long) index];
            if ((image->columns % 4) >= 1)

              {
                index=ConstrainColormapIndex(image,(*p >> 4) & 0x3);
                indexes[x]=index;
                *q++=image->colormap[(long) index];
                if ((image->columns % 4) >= 2)

                  {
                    index=ConstrainColormapIndex(image,(*p >> 2) & 0x3);
                    indexes[x]=index;
                    *q++=image->colormap[(long) index];
                  }
              }
            p++;
          }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
        /*         if (image->previous == (Image *) NULL)
                   if (QuantumTick(y,image->rows) != MagickFalse)
                   ProgressMonitor(LoadImageText,image->rows-y-1,image->rows);*/
        break;
      }

    case 4:  /* Convert PseudoColor scanline. */
      {
        q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
        if (q == (PixelPacket *) NULL)
          break;
        indexes=GetAuthenticIndexQueue(image);
        for (x=0; x < ((long) image->columns-1); x+=2)
          {
            index=ConstrainColormapIndex(image,(*p >> 4) & 0xf);
            indexes[x]=index;
            *q++=image->colormap[(long) index];
            index=ConstrainColormapIndex(image,(*p) & 0xf);
            indexes[x+1]=index;
            *q++=image->colormap[(long) index];
            p++;
          }
        if ((image->columns % 2) != 0)
          {
            index=ConstrainColormapIndex(image,(*p >> 4) & 0xf);
            indexes[x]=index;
            *q++=image->colormap[(long) index];
            p++;
          }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
        /*         if (image->previous == (Image *) NULL)
                   if (QuantumTick(y,image->rows) != MagickFalse)
                   ProgressMonitor(LoadImageText,image->rows-y-1,image->rows);*/
        break;
      }
    case 8: /* Convert PseudoColor scanline. */
      {
        q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
        if (q == (PixelPacket *) NULL) break;
        indexes=GetAuthenticIndexQueue(image);

        for (x=0; x < (long) image->columns; x++)
          {
            index=ConstrainColormapIndex(image,*p);
            indexes[x]=index;
            *q++=image->colormap[(long) index];
            p++;
          }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
        /*           if (image->previous == (Image *) NULL)
                     if (QuantumTick(y,image->rows) != MagickFalse)
                     ProgressMonitor(LoadImageText,image->rows-y-1,image->rows);*/
      }
      break;

    }
}

/*
   Compute the number of colors in Grayed R[i]=G[i]=B[i] image
*/
static int GetCutColors(Image *image)
{
  ExceptionInfo
    *exception;

  long
    x,
    y;

  PixelPacket
    *q;

  Quantum
    intensity,
    scale_intensity;

  exception=(&image->exception);
  intensity=0;
  scale_intensity=ScaleCharToQuantum(16);
  for (y=0; y < (long) image->rows; y++)
  {
    q=GetAuthenticPixels(image,0,y,image->columns,1,exception);
    for (x=0; x < (long) image->columns; x++)
    {
      if (intensity < q->red)
        intensity=q->red;
      if (intensity >= scale_intensity)
        return(255);
      q++;
    }
  }
  if (intensity < ScaleCharToQuantum(2))
    return(2);
  if (intensity < ScaleCharToQuantum(16))
    return(16);
  return((int) intensity);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d C U T I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadCUTImage() reads an CUT X image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadCUTImage method is:
%
%      Image *ReadCUTImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadCUTImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  Image *image,*palette;
  ImageInfo *clone_info;
  MagickBooleanType status;

  MagickOffsetType
    offset;

  unsigned long EncodedByte;
  unsigned char RunCount,RunValue,RunCountMasked;
  CUTHeader  Header;
  CUTPalHeader PalHeader;
  long depth;
  long i,j;
  long ldblk;
  unsigned char *BImgBuff=NULL,*ptrB;
  PixelPacket *q;

  ssize_t
    count;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  if (image_info->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  image=AcquireImage(image_info);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  /*
    Read CUT image.
  */
  palette=NULL;
  clone_info=NULL;
  Header.Width=ReadBlobLSBShort(image);
  Header.Height=ReadBlobLSBShort(image);
  Header.Reserved=ReadBlobLSBShort(image);

  if (Header.Width==0 || Header.Height==0 || Header.Reserved!=0)
    CUT_KO:  ThrowReaderException(CorruptImageError,"ImproperImageHeader");

  /*---This code checks first line of image---*/
  EncodedByte=ReadBlobLSBShort(image);
  RunCount=(unsigned char) ReadBlobByte(image);
  RunCountMasked=RunCount & 0x7F;
  ldblk=0;
  while((int) RunCountMasked!=0)  /*end of line?*/
    {
      i=1;
      if((int) RunCount<0x80) i=(long) RunCountMasked;
      offset=SeekBlob(image,TellBlob(image)+i,SEEK_SET);
      if (offset < 0)
        ThrowReaderException(CorruptImageError,"ImproperImageHeader");
      if(EOFBlob(image) != MagickFalse) goto CUT_KO;  /*wrong data*/
      EncodedByte-=i+1;
      ldblk+=(long) RunCountMasked;

      RunCount=(unsigned char) ReadBlobByte(image);
      if(EOFBlob(image) != MagickFalse)  goto CUT_KO;  /*wrong data: unexpected eof in line*/
      RunCountMasked=RunCount & 0x7F;
    }
  if(EncodedByte!=1) goto CUT_KO;  /*wrong data: size incorrect*/
  i=0;        /*guess a number of bit planes*/
  if(ldblk==(int) Header.Width)   i=8;
  if(2*ldblk==(int) Header.Width) i=4;
  if(8*ldblk==(int) Header.Width) i=1;
  if(i==0) goto CUT_KO;    /*wrong data: incorrect bit planes*/
  depth=i;

  image->columns=Header.Width;
  image->rows=Header.Height;
  image->depth=8;
  image->colors=(unsigned long) (GetQuantumRange(1UL*i)+1);

  if (image_info->ping) goto Finish;

  /* ----- Do something with palette ----- */
  if ((clone_info=CloneImageInfo(image_info)) == NULL) goto NoPalette;


  i=(long) strlen(clone_info->filename);
  j=i;
  while(--i>0)
    {
      if(clone_info->filename[i]=='.')
        {
          break;
        }
      if(clone_info->filename[i]=='/' || clone_info->filename[i]=='\\' ||
         clone_info->filename[i]==':' )
        {
          i=j;
          break;
        }
    }

  (void) CopyMagickString(clone_info->filename+i,".PAL",(size_t)
    (MaxTextExtent-i));
  if((clone_info->file=OpenMagickStream(clone_info->filename,"rb"))==NULL)
    {
      (void) CopyMagickString(clone_info->filename+i,".pal",(size_t)
        (MaxTextExtent-i));
      if((clone_info->file=OpenMagickStream(clone_info->filename,"rb"))==NULL)
        {
          clone_info->filename[i]='\0';
          if((clone_info->file=OpenMagickStream(clone_info->filename,"rb"))==NULL)
            {
              clone_info=DestroyImageInfo(clone_info);
              clone_info=NULL;
              goto NoPalette;
            }
        }
    }

  if( (palette=AcquireImage(clone_info))==NULL ) goto NoPalette;
  status=OpenBlob(clone_info,palette,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
    ErasePalette:
      palette=DestroyImage(palette);
      palette=NULL;
      goto NoPalette;
    }


  if(palette!=NULL)
    {
      count=ReadBlob(palette,2,(unsigned char *) PalHeader.FileId);
      if(strncmp(PalHeader.FileId,"AH",2) != 0) goto ErasePalette;
      PalHeader.Version=ReadBlobLSBShort(palette);
      PalHeader.Size=ReadBlobLSBShort(palette);
      PalHeader.FileType=(char) ReadBlobByte(palette);
      PalHeader.SubType=(char) ReadBlobByte(palette);
      PalHeader.BoardID=ReadBlobLSBShort(palette);
      PalHeader.GraphicsMode=ReadBlobLSBShort(palette);
      PalHeader.MaxIndex=ReadBlobLSBShort(palette);
      PalHeader.MaxRed=ReadBlobLSBShort(palette);
      PalHeader.MaxGreen=ReadBlobLSBShort(palette);
      PalHeader.MaxBlue=ReadBlobLSBShort(palette);
      count=ReadBlob(palette,20,(unsigned char *) PalHeader.PaletteId);

      if(PalHeader.MaxIndex<1) goto ErasePalette;
      image->colors=PalHeader.MaxIndex+1;
      if (AcquireImageColormap(image,image->colors) == MagickFalse) goto NoMemory;

      if(PalHeader.MaxRed==0) PalHeader.MaxRed=(unsigned int) QuantumRange;  /*avoid division by 0*/
      if(PalHeader.MaxGreen==0) PalHeader.MaxGreen=(unsigned int) QuantumRange;
      if(PalHeader.MaxBlue==0) PalHeader.MaxBlue=(unsigned int) QuantumRange;

      for(i=0;i<=(int) PalHeader.MaxIndex;i++)
        {      /*this may be wrong- I don't know why is palette such strange*/
          j=(long) TellBlob(palette);
          if((j % 512)>512-6)
            {
              j=((j / 512)+1)*512;
              offset=SeekBlob(palette,j,SEEK_SET);
              if (offset < 0)
                ThrowReaderException(CorruptImageError,"ImproperImageHeader");
            }
          image->colormap[i].red=(Quantum) ReadBlobLSBShort(palette);
          if (QuantumRange != (Quantum) PalHeader.MaxRed)
            {
              image->colormap[i].red=RoundToQuantum(((double)
                image->colormap[i].red*QuantumRange+(PalHeader.MaxRed>>1))/
                PalHeader.MaxRed);
            }
          image->colormap[i].green=(Quantum) ReadBlobLSBShort(palette);
          if (QuantumRange != (Quantum) PalHeader.MaxGreen)
            {
              image->colormap[i].green=RoundToQuantum
                (((double) image->colormap[i].green*QuantumRange+(PalHeader.MaxGreen>>1))/PalHeader.MaxGreen);
            }
          image->colormap[i].blue=(Quantum) ReadBlobLSBShort(palette);
          if (QuantumRange != (Quantum) PalHeader.MaxBlue)
            {
              image->colormap[i].blue=RoundToQuantum
                (((double)image->colormap[i].blue*QuantumRange+(PalHeader.MaxBlue>>1))/PalHeader.MaxBlue);
            }

        }
    }



 NoPalette:
  if(palette==NULL)
    {

      image->colors=256;
      if (AcquireImageColormap(image,image->colors) == MagickFalse)
        {
        NoMemory:
          ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
            }

      for (i=0; i < (long)image->colors; i++)
        {
          image->colormap[i].red=ScaleCharToQuantum((unsigned char) i);
          image->colormap[i].green=ScaleCharToQuantum((unsigned char) i);
          image->colormap[i].blue=ScaleCharToQuantum((unsigned char) i);
        }
    }


  /* ----- Load RLE compressed raster ----- */
  BImgBuff=(unsigned char *) AcquireQuantumMemory((size_t) ldblk,
    sizeof(*BImgBuff));  /*Ldblk was set in the check phase*/
  if(BImgBuff==NULL) goto NoMemory;

  offset=SeekBlob(image,6 /*sizeof(Header)*/,SEEK_SET);
  if (offset < 0)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  for (i=0; i < (int) Header.Height; i++)
  {
      EncodedByte=ReadBlobLSBShort(image);

      ptrB=BImgBuff;
      j=ldblk;

      RunCount=(unsigned char) ReadBlobByte(image);
      RunCountMasked=RunCount & 0x7F;

      while((int) RunCountMasked!=0)
        {
          if((long) RunCountMasked>j)
            {    /*Wrong Data*/
              RunCountMasked=(unsigned char) j;
              if(j==0)
                {
                  break;
                }
            }

          if((int) RunCount>0x80)
            {
              RunValue=(unsigned char) ReadBlobByte(image);
              (void) ResetMagickMemory(ptrB,(int) RunValue,(size_t) RunCountMasked);
            }
          else {
            count=ReadBlob(image,(size_t) RunCountMasked,ptrB);
          }

          ptrB+=(int) RunCountMasked;
          j-=(int) RunCountMasked;

          if (EOFBlob(image) != MagickFalse) goto Finish;  /* wrong data: unexpected eof in line */
          RunCount=(unsigned char) ReadBlobByte(image);
          RunCountMasked=RunCount & 0x7F;
        }

      InsertRow(depth,BImgBuff,i,image);
    }


  /*detect monochrome image*/

  if(palette==NULL)
    {    /*attempt to detect binary (black&white) images*/
      if ((image->storage_class == PseudoClass) &&
          (IsGrayImage(image,&image->exception) != MagickFalse))
        {
          if(GetCutColors(image)==2)
            {
              for (i=0; i < (long)image->colors; i++)
                {
                  register Quantum
                    sample;
                  sample=ScaleCharToQuantum((unsigned char) i);
                  if(image->colormap[i].red!=sample) goto Finish;
                  if(image->colormap[i].green!=sample) goto Finish;
                  if(image->colormap[i].blue!=sample) goto Finish;
                }

              image->colormap[1].red=image->colormap[1].green=image->colormap[1].blue=(Quantum) QuantumRange;
              for (i=0; i < (long)image->rows; i++)
                {
                  q=QueueAuthenticPixels(image,0,i,image->columns,1,exception);
                  for (j=0; j < (long)image->columns; j++)
                    {
                      if(q->red==ScaleCharToQuantum(1))
                        {
                          q->red=q->green=q->blue=(Quantum) QuantumRange;
                        }
                      q++;
                    }
                  if (SyncAuthenticPixels(image,exception) == MagickFalse) goto Finish;
                }
            }
        }
    }

 Finish:
  if (BImgBuff != NULL)
    BImgBuff=(unsigned char *) RelinquishMagickMemory(BImgBuff);
  if (palette != NULL)
    palette=DestroyImage(palette);
  if (clone_info != NULL)
    clone_info=DestroyImageInfo(clone_info);
  if (EOFBlob(image) != MagickFalse)
    ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
      image->filename);
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r C U T I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterCUTImage() adds attributes for the CUT image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterCUTImage method is:
%
%      unsigned long RegisterCUTImage(void)
%
*/
ModuleExport unsigned long RegisterCUTImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("CUT");
  entry->decoder=(DecodeImageHandler *) ReadCUTImage;
  entry->seekable_stream=MagickTrue;
  entry->description=ConstantString("DR Halo");
  entry->module=ConstantString("CUT");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r C U T I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterCUTImage() removes format registrations made by the
%  CUT module from the list of supported formats.
%
%  The format of the UnregisterCUTImage method is:
%
%      UnregisterCUTImage(void)
%
*/
ModuleExport void UnregisterCUTImage(void)
{
  (void) UnregisterMagickInfo("CUT");
}
