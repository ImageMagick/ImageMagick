/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                  M   M   AAA   TTTTT  L       AAA   BBBB                    %
%                  MM MM  A   A    T    L      A   A  B   B                   %
%                  M M M  AAAAA    T    L      AAAAA  BBBB                    %
%                  M   M  A   A    T    L      A   A  B   B                   %
%                  M   M  A   A    T    LLLLL  A   A  BBBB                    %
%                                                                             %
%                                                                             %
%                        Read MATLAB Image Format                             %
%                                                                             %
%                              Software Design                                %
%                              Jaroslav Fojtik                                %
%                                2001-2008                                    %
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
#include "magick/color-private.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/quantum-private.h"
#include "magick/option.h"
#include "magick/resource_.h"
#include "magick/shear.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"
#include "magick/transform.h"
#if defined(MAGICKCORE_ZLIB_DELEGATE)
 #include "zlib.h"
#endif

/*
  Forward declaration.
*/
static MagickBooleanType
  WriteMATImage(const ImageInfo *,Image *);


/* Auto coloring method, sorry this creates some artefact inside data
MinReal+j*MaxComplex = red  MaxReal+j*MaxComplex = black
MinReal+j*0 = white          MaxReal+j*0 = black
MinReal+j*MinComplex = blue  MaxReal+j*MinComplex = black
*/

typedef struct
{
  char identific[124];
  unsigned short Version;
  char EndianIndicator[2];
  unsigned long DataType;
  unsigned long ObjectSize;
  unsigned long unknown1;
  unsigned long unknown2;

  unsigned short unknown5;
  unsigned char StructureFlag;
  unsigned char StructureClass;
  unsigned long unknown3;
  unsigned long unknown4;
  unsigned long DimFlag;

  unsigned long SizeX;
  unsigned long SizeY;
  unsigned short Flag1;
  unsigned short NameFlag;
}
MATHeader;

static const char *MonthsTab[12]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};
static const char *DayOfWTab[7]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
static const char *OsDesc=
#ifdef __WIN32__
    "PCWIN";
#else
 #ifdef __APPLE__
    "MAC";
 #else
    "LNX86";
 #endif
#endif

typedef enum
  {
    miINT8 = 1,			/* 8 bit signed */
    miUINT8,			/* 8 bit unsigned */
    miINT16,			/* 16 bit signed */
    miUINT16,			/* 16 bit unsigned */
    miINT32,			/* 32 bit signed */
    miUINT32,			/* 32 bit unsigned */
    miSINGLE,			/* IEEE 754 single precision float */
    miRESERVE1,
    miDOUBLE,			/* IEEE 754 double precision float */
    miRESERVE2,
    miRESERVE3,
    miINT64,			/* 64 bit signed */
    miUINT64,			/* 64 bit unsigned */
    miMATRIX,		        /* MATLAB array */
    miCOMPRESSED,	        /* Compressed Data */
    miUTF8,		        /* Unicode UTF-8 Encoded Character Data */
    miUTF16,		        /* Unicode UTF-16 Encoded Character Data */
    miUTF32			/* Unicode UTF-32 Encoded Character Data */
  } mat5_data_type;

typedef enum
  {
    mxCELL_CLASS=1,		/* cell array */
    mxSTRUCT_CLASS,		/* structure */
    mxOBJECT_CLASS,		/* object */
    mxCHAR_CLASS,		/* character array */
    mxSPARSE_CLASS,		/* sparse array */
    mxDOUBLE_CLASS,		/* double precision array */
    mxSINGLE_CLASS,		/* single precision floating point */
    mxINT8_CLASS,		/* 8 bit signed integer */
    mxUINT8_CLASS,		/* 8 bit unsigned integer */
    mxINT16_CLASS,		/* 16 bit signed integer */
    mxUINT16_CLASS,		/* 16 bit unsigned integer */
    mxINT32_CLASS,		/* 32 bit signed integer */
    mxUINT32_CLASS,		/* 32 bit unsigned integer */
    mxINT64_CLASS,		/* 64 bit signed integer */
    mxUINT64_CLASS,		/* 64 bit unsigned integer */
    mxFUNCTION_CLASS            /* Function handle */
  } arrayclasstype;

#define FLAG_COMPLEX 0x8
#define FLAG_GLOBAL  0x4
#define FLAG_LOGICAL 0x2

static const QuantumType z2qtype[4] = {GrayQuantum, BlueQuantum, GreenQuantum, RedQuantum};


static void InsertComplexDoubleRow(double *p, int y, Image * image, double MinVal,
                                  double MaxVal)
{
  ExceptionInfo
    *exception;

  double f;
  int x;
  register PixelPacket *q;

  if (MinVal == 0)
    MinVal = -1;
  if (MaxVal == 0)
    MaxVal = 1;

  exception=(&image->exception);
  q = QueueAuthenticPixels(image, 0, y, image->columns, 1,exception);
  if (q == (PixelPacket *) NULL)
    return;
  for (x = 0; x < (long) image->columns; x++)
  {
    if (*p > 0)
    {
      f = (*p / MaxVal) * (QuantumRange - q->red);
      if (f + q->red > QuantumRange)
        q->red = QuantumRange;
      else
        q->red += (int) f;
      if ((int) f / 2.0 > q->green)
        q->green = q->blue = 0;
      else
        q->green = q->blue -= (int) (f / 2.0);
    }
    if (*p < 0)
    {
      f = (*p / MaxVal) * (QuantumRange - q->blue);
      if (f + q->blue > QuantumRange)
        q->blue = QuantumRange;
      else
        q->blue += (int) f;
      if ((int) f / 2.0 > q->green)
        q->green = q->red = 0;
      else
        q->green = q->red -= (int) (f / 2.0);
    }
    p++;
    q++;
  }
  if (!SyncAuthenticPixels(image,exception))
    return;
  /*          if (image->previous == (Image *) NULL)
     if (QuantumTick(y,image->rows))
     MagickMonitor(LoadImageText,image->rows-y-1,image->rows); */
  return;
}


static void InsertComplexFloatRow(float *p, int y, Image * image, double MinVal,
                                  double MaxVal)
{
  ExceptionInfo
    *exception;

  double f;
  int x;
  register PixelPacket *q;

  if (MinVal == 0)
    MinVal = -1;
  if (MaxVal == 0)
    MaxVal = 1;

  exception=(&image->exception);
  q = QueueAuthenticPixels(image, 0, y, image->columns, 1,exception);
  if (q == (PixelPacket *) NULL)
    return;
  for (x = 0; x < (long) image->columns; x++)
  {
    if (*p > 0)
    {
      f = (*p / MaxVal) * (QuantumRange - q->red);
      if (f + q->red > QuantumRange)
        q->red = QuantumRange;
      else
        q->red += (int) f;
      if ((int) f / 2.0 > q->green)
        q->green = q->blue = 0;
      else
        q->green = q->blue -= (int) (f / 2.0);
    }
    if (*p < 0)
    {
      f = (*p / MaxVal) * (QuantumRange - q->blue);
      if (f + q->blue > QuantumRange)
        q->blue = QuantumRange;
      else
        q->blue += (int) f;
      if ((int) f / 2.0 > q->green)
        q->green = q->red = 0;
      else
        q->green = q->red -= (int) (f / 2.0);
    }
    p++;
    q++;
  }
  if (!SyncAuthenticPixels(image,exception))
    return;
  /*          if (image->previous == (Image *) NULL)
     if (QuantumTick(y,image->rows))
     MagickMonitor(LoadImageText,image->rows-y-1,image->rows); */
  return;
}


/************** READERS ******************/

/* This function reads one block of floats*/
static void ReadBlobFloatsLSB(Image * image, size_t len, float *data)
{
  while (len >= 4)
  {
    *data++ = ReadBlobFloat(image);
    len -= sizeof(float);
  }
  if (len > 0)
    (void) SeekBlob(image, len, SEEK_CUR);
}

static void ReadBlobFloatsMSB(Image * image, size_t len, float *data)
{
  while (len >= 4)
  {
    *data++ = ReadBlobFloat(image);
    len -= sizeof(float);
  }
  if (len > 0)
    (void) SeekBlob(image, len, SEEK_CUR);
}

/* This function reads one block of doubles*/
static void ReadBlobDoublesLSB(Image * image, size_t len, double *data)
{
  while (len >= 8)
  {
    *data++ = ReadBlobDouble(image);
    len -= sizeof(double);
  }
  if (len > 0)
    (void) SeekBlob(image, len, SEEK_CUR);
}

static void ReadBlobDoublesMSB(Image * image, size_t len, double *data)
{
  while (len >= 8)
  {
    *data++ = ReadBlobDouble(image);
    len -= sizeof(double);
  }
  if (len > 0)
    (void) SeekBlob(image, len, SEEK_CUR);
}

/* Calculate minimum and maximum from a given block of data */
static void CalcMinMax(Image *image, int endian_indicator, int SizeX, int SizeY, unsigned long CellType, unsigned ldblk, void *BImgBuff, double *Min, double *Max)
{
MagickOffsetType filepos;
int i, x;
void (*ReadBlobDoublesXXX)(Image * image, size_t len, double *data);
void (*ReadBlobFloatsXXX)(Image * image, size_t len, float *data);
double *dblrow;
float *fltrow;

  if (endian_indicator == LSBEndian)
  {    
    ReadBlobDoublesXXX = ReadBlobDoublesLSB;
    ReadBlobFloatsXXX = ReadBlobFloatsLSB;   
  } 
  else		/* MI */
  {    
    ReadBlobDoublesXXX = ReadBlobDoublesMSB;
    ReadBlobFloatsXXX = ReadBlobFloatsMSB;   
  }

  filepos = TellBlob(image);	   /* Please note that file seeking occurs only in the case of doubles */
  for (i = 0; i < SizeY; i++)
  {
    if (CellType==miDOUBLE)
    {
      ReadBlobDoublesXXX(image, ldblk, (double *)BImgBuff);
      dblrow = (double *)BImgBuff;
      if (i == 0)
      {
        *Min = *Max = *dblrow;
      }
      for (x = 0; x < SizeX; x++)
      {
        if (*Min > *dblrow)
          *Min = *dblrow;
        if (*Max < *dblrow)
          *Max = *dblrow;
        dblrow++;
      }
    }
    if (CellType==miSINGLE)
    {
      ReadBlobFloatsXXX(image, ldblk, (float *)BImgBuff);
      fltrow = (float *)BImgBuff;
      if (i == 0)
      {
        *Min = *Max = *fltrow;
      }
    for (x = 0; x < (long) SizeX; x++)
      {
        if (*Min > *fltrow)
          *Min = *fltrow;
        if (*Max < *fltrow)
          *Max = *fltrow;
        fltrow++;
      }
    }
  }
  (void) SeekBlob(image, filepos, SEEK_SET);
}


static void FixSignedValues(PixelPacket *q, int y)
{
  while(y-->0)
  {
     /* Please note that negative values will overflow
        Q=8; QuantumRange=255: <0;127> + 127+1 = <128; 255> 
		       <-1;-128> + 127+1 = <0; 127> */
    q->red += QuantumRange/2 + 1;
    q->green += QuantumRange/ + 1;
    q->blue += QuantumRange/ + 1;
    q++;
  }
}


/** Fix whole row of logical/binary data. It means pack it. */
static void FixLogical(unsigned char *Buff,int ldblk)
{
unsigned char mask=128;
unsigned char *BuffL = Buff;
unsigned char val = 0;

  while(ldblk-->0)
  {
    if(*Buff++ != 0)
      val |= mask;    

    mask >>= 1;
    if(mask==0)
    {
      *BuffL++ = val;
      val = 0;
      mask = 128;
    }   
      
  }
  *BuffL = val;
}

#if defined(MAGICKCORE_ZLIB_DELEGATE)
static voidpf AcquireZIPMemory(voidpf context,unsigned int items,
  unsigned int size)
{
  (void) context;
  return((voidpf) AcquireQuantumMemory(items,size));
}

static void RelinquishZIPMemory(voidpf context,voidpf memory)
{
  (void) context;
  memory=RelinquishMagickMemory(memory);
}
#endif

/** This procedure decompreses an image block for a new MATLAB format. */
static Image *DecompressBlock(Image *orig, MagickOffsetType Size, ImageInfo *clone_info, ExceptionInfo *exception)
{
#if defined(MAGICKCORE_ZLIB_DELEGATE)

Image *image2;
void *CacheBlock, *DecompressBlock;
z_stream zip_info;
FILE *mat_file;
size_t magick_size;

int status;

  if(clone_info==NULL) return NULL;
  if(clone_info->file)		/* Close file opened from previous transaction. */
  {
    fclose(clone_info->file);
    clone_info->file = NULL;
    (void) unlink(clone_info->filename);
  }

  CacheBlock = AcquireQuantumMemory((size_t)((Size<16384)?Size:16384),sizeof(unsigned char *));
  if(CacheBlock==NULL) return NULL;
  DecompressBlock = AcquireQuantumMemory((size_t)(4096),sizeof(unsigned char *));
  if(DecompressBlock==NULL) 
  {
    RelinquishMagickMemory(CacheBlock);    
    return NULL;
  }

  mat_file = fdopen(AcquireUniqueFileResource(clone_info->filename),"w");
  if(!mat_file)
  {
    RelinquishMagickMemory(CacheBlock);
    RelinquishMagickMemory(DecompressBlock);
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),"Gannot create file stream for PS image");
    return NULL;
  }

  zip_info.zalloc=AcquireZIPMemory;
  zip_info.zfree=RelinquishZIPMemory;
  zip_info.opaque = (voidpf) NULL;
  inflateInit(&zip_info);
  /* zip_info.next_out = 8*4;*/

  zip_info.avail_in = 0;
  zip_info.total_out = 0;
  while(Size>0 && !EOFBlob(orig))
  {    
    magick_size = ReadBlob(orig, (Size<16384)?Size:16384, (unsigned char *) CacheBlock);
    zip_info.next_in = (Bytef *) CacheBlock;
    zip_info.avail_in = (uInt) magick_size;    

    while(zip_info.avail_in>0)
    {
      zip_info.avail_out = 4096;    
      zip_info.next_out = (Bytef *) DecompressBlock;
      status = inflate(&zip_info,Z_NO_FLUSH);      
      fwrite(DecompressBlock, 4096-zip_info.avail_out, 1, mat_file);

      if(status == Z_STREAM_END) goto DblBreak;
    }

    Size -= magick_size;
  }
DblBreak:
 
  (void)fclose(mat_file);
  RelinquishMagickMemory(CacheBlock);
  RelinquishMagickMemory(DecompressBlock);

  if((clone_info->file=fopen(clone_info->filename,"rb"))==NULL) goto UnlinkFile;
  if( (image2 = AcquireImage(clone_info))==NULL ) goto EraseFile;  
  status = OpenBlob(clone_info,image2,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
  {
    DeleteImageFromList(&image2);    
EraseFile:
    fclose(clone_info->file);
    clone_info->file = NULL;
UnlinkFile:
    (void) unlink(clone_info->filename);
    return NULL; 
  }

  return image2;
#else
  (void) orig;
  (void) Size;
  (void) clone_info;
  (void) exception;
  return NULL;
#endif
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d M A T L A B i m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadMATImage() reads an MAT X image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadMATImage method is:
%
%      Image *ReadMATImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image:  Method ReadMATImage returns a pointer to the image after
%      reading. A null image is returned if there is a memory shortage or if
%      the image cannot be read.
%
%    o image_info: Specifies a pointer to a ImageInfo structure.
%
%    o exception: return any errors or warnings in this structure.
%
*/

static inline size_t MagickMin(const size_t x,const size_t y)
{
  if (x < y)
    return(x);
  return(y);
}

static Image *ReadMATImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  Image *image, *image2=NULL,
   *rotated_image;
  PixelPacket *q;

  unsigned int status;
  MATHeader MATLAB_HDR;
  unsigned long size;  
  unsigned long CellType;
  QuantumInfo *quantum_info;
  ImageInfo *clone_info;
  int i;
  long ldblk;
  unsigned char *BImgBuff = NULL;
  double MinVal, MaxVal;
  unsigned long Unknown6;
  unsigned z;
  int logging;
  int sample_size;
  MagickOffsetType filepos=0x80;
  BlobInfo *blob;
  
  unsigned int (*ReadBlobXXXLong)(Image *image);
  unsigned short (*ReadBlobXXXShort)(Image *image);
  void (*ReadBlobDoublesXXX)(Image * image, size_t len, double *data);
  void (*ReadBlobFloatsXXX)(Image * image, size_t len, float *data);


  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  logging = LogMagickEvent(CoderEvent,GetMagickModule(),"enter"); 

  /*
     Open image file.
   */
  image = AcquireImage(image_info);

  status = OpenBlob(image_info, image, ReadBinaryBlobMode, exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  /*
     Read MATLAB image.
   */
  clone_info=CloneImageInfo(image_info);
  if(ReadBlob(image,124,(unsigned char *) &MATLAB_HDR.identific) != 124)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  MATLAB_HDR.Version = ReadBlobLSBShort(image);
  if(ReadBlob(image,2,(unsigned char *) &MATLAB_HDR.EndianIndicator) != 2)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");

  if (logging) (void)LogMagickEvent(CoderEvent,GetMagickModule(),"  Endian %c%c",
        MATLAB_HDR.EndianIndicator[0],MATLAB_HDR.EndianIndicator[1]);
  if (!strncmp(MATLAB_HDR.EndianIndicator, "IM", 2))
  {
    ReadBlobXXXLong = ReadBlobLSBLong;
    ReadBlobXXXShort = ReadBlobLSBShort;
    ReadBlobDoublesXXX = ReadBlobDoublesLSB;
    ReadBlobFloatsXXX = ReadBlobFloatsLSB;
    image->endian = LSBEndian;
  } 
  else if (!strncmp(MATLAB_HDR.EndianIndicator, "MI", 2))
  {
    ReadBlobXXXLong = ReadBlobMSBLong;
    ReadBlobXXXShort = ReadBlobMSBShort;
    ReadBlobDoublesXXX = ReadBlobDoublesMSB;
    ReadBlobFloatsXXX = ReadBlobFloatsMSB;
    image->endian = MSBEndian;
  }
  else 
    goto MATLAB_KO;    /* unsupported endian */

  if (strncmp(MATLAB_HDR.identific, "MATLAB", 6))
MATLAB_KO: ThrowReaderException(CorruptImageError,"ImproperImageHeader");

  filepos = TellBlob(image);
  while(!EOFBlob(image)) /* object parser loop */
  {
    (void) SeekBlob(image,filepos,SEEK_SET);
    /* printf("pos=%X\n",TellBlob(image)); */

    MATLAB_HDR.DataType = ReadBlobXXXLong(image);
    if(EOFBlob(image)) break;
    MATLAB_HDR.ObjectSize = ReadBlobXXXLong(image);
    if(EOFBlob(image)) break;
    filepos += MATLAB_HDR.ObjectSize + 4 + 4;

    image2 = image;
#if defined(MAGICKCORE_ZLIB_DELEGATE)
    if(MATLAB_HDR.DataType == miCOMPRESSED)
    {
      image2 = DecompressBlock(image,MATLAB_HDR.ObjectSize,clone_info,exception);
      if(image2==NULL) continue;
      MATLAB_HDR.DataType = ReadBlobXXXLong(image2); /* replace compressed object type. */
    }
#endif    

    if(MATLAB_HDR.DataType!=miMATRIX) continue;  /* skip another objects. */
 
    MATLAB_HDR.unknown1 = ReadBlobXXXLong(image2);
    MATLAB_HDR.unknown2 = ReadBlobXXXLong(image2);  

    MATLAB_HDR.unknown5 = ReadBlobXXXLong(image2);
    MATLAB_HDR.StructureClass = MATLAB_HDR.unknown5 & 0xFF;
    MATLAB_HDR.StructureFlag = (MATLAB_HDR.unknown5>>8) & 0xFF;  

    MATLAB_HDR.unknown3 = ReadBlobXXXLong(image2);
    if(image!=image2)
      MATLAB_HDR.unknown4 = ReadBlobXXXLong(image2);	/* ??? don't understand why ?? */
    MATLAB_HDR.unknown4 = ReadBlobXXXLong(image2);
    MATLAB_HDR.DimFlag = ReadBlobXXXLong(image2);
    MATLAB_HDR.SizeX = ReadBlobXXXLong(image2);
    MATLAB_HDR.SizeY = ReadBlobXXXLong(image2);  
   

    switch(MATLAB_HDR.DimFlag)
    {     
      case  8: z=1; break;			/* 2D matrix*/
      case 12: z = ReadBlobXXXLong(image2);	/* 3D matrix RGB*/
  	       Unknown6 = ReadBlobXXXLong(image2);
	       if(z!=3) ThrowReaderException(CoderError, "MultidimensionalMatricesAreNotSupported");
	       break;
      default: ThrowReaderException(CoderError, "MultidimensionalMatricesAreNotSupported");
    }  

    MATLAB_HDR.Flag1 = ReadBlobXXXShort(image2);
    MATLAB_HDR.NameFlag = ReadBlobXXXShort(image2);

    if (logging) (void)LogMagickEvent(CoderEvent,GetMagickModule(),
          "MATLAB_HDR.StructureClass %d",MATLAB_HDR.StructureClass);
    if (MATLAB_HDR.StructureClass != mxCHAR_CLASS && 
        MATLAB_HDR.StructureClass != mxSINGLE_CLASS &&		/* float + complex float */
        MATLAB_HDR.StructureClass != mxDOUBLE_CLASS &&		/* double + complex double */
        MATLAB_HDR.StructureClass != mxINT8_CLASS &&
        MATLAB_HDR.StructureClass != mxUINT8_CLASS &&		/* uint8 + uint8 3D */
        MATLAB_HDR.StructureClass != mxINT16_CLASS &&
        MATLAB_HDR.StructureClass != mxUINT16_CLASS &&		/* uint16 + uint16 3D */
        MATLAB_HDR.StructureClass != mxINT32_CLASS &&
        MATLAB_HDR.StructureClass != mxUINT32_CLASS &&		/* uint32 + uint32 3D */
        MATLAB_HDR.StructureClass != mxINT64_CLASS &&
        MATLAB_HDR.StructureClass != mxUINT64_CLASS)		/* uint64 + uint64 3D */
      ThrowReaderException(CoderError,"UnsupportedCellTypeInTheMatrix");

    switch (MATLAB_HDR.NameFlag)
    {
      case 0:
        size = ReadBlobXXXLong(image2);	/* Object name string size */
        size = 4 * (long) ((size + 3 + 1) / 4);
        (void) SeekBlob(image2, size, SEEK_CUR);
        break;
      case 1:
      case 2:
      case 3:
      case 4:
        (void) ReadBlob(image2, 4, (unsigned char *) &size); /* Object name string */
        break;
      default:
        goto MATLAB_KO;
    }

    CellType = ReadBlobXXXLong(image2);    /* Additional object type */
    if (logging) (void)LogMagickEvent(CoderEvent,GetMagickModule(),
          "MATLAB_HDR.CellType: %ld",CellType);
  
    (void) ReadBlob(image2, 4, (unsigned char *) &size);     /* data size */

      /* Image is gray when no complex flag is set and 2D Matrix */
    if ((MATLAB_HDR.DimFlag == 8) &&
        ((MATLAB_HDR.StructureFlag & FLAG_COMPLEX) == 0))
      image->type=GrayscaleType;

    switch (CellType)
    {
      case miINT8:
      case miUINT8:
        sample_size = 8;
        if(MATLAB_HDR.StructureFlag & FLAG_LOGICAL) 
          image->depth = 1;
        else
          image->depth = 8;         /* Byte type cell */
        ldblk = (long) MATLAB_HDR.SizeX;      
        break;
      case miINT16:
      case miUINT16:
        sample_size = 16;
        image->depth = 16;        /* Word type cell */
        ldblk = (long) (2 * MATLAB_HDR.SizeX);
        break;
      case miINT32:
      case miUINT32:
        sample_size = 32;
        image->depth = 32;        /* Dword type cell */
        ldblk = (long) (4 * MATLAB_HDR.SizeX);      
        break;
      case miINT64:
      case miUINT64:
        sample_size = 64;
        image->depth = 64;        /* Qword type cell */
        ldblk = (long) (8 * MATLAB_HDR.SizeX);      
        break;   
      case miSINGLE:
        sample_size = 32;
        image->depth = 32;        /* double type cell */
        (void) SetImageOption(clone_info,"quantum:format","floating-point");
        if (MATLAB_HDR.StructureFlag & FLAG_COMPLEX)
	{					    /* complex float type cell */
	}
        ldblk = (long) (4 * MATLAB_HDR.SizeX);
        break;
      case miDOUBLE:
        sample_size = 64; 
        image->depth = 64;        /* double type cell */
        (void) SetImageOption(clone_info,"quantum:format","floating-point");
        if (sizeof(double) != 8)
          ThrowReaderException(CoderError, "IncompatibleSizeOfDouble");
        if (MATLAB_HDR.StructureFlag & FLAG_COMPLEX)
	{                         /* complex double type cell */        
	}
        ldblk = (long) (8 * MATLAB_HDR.SizeX);
        break;
      default:
        ThrowReaderException(CoderError, "UnsupportedCellTypeInTheMatrix");
    }
    image->columns = MATLAB_HDR.SizeX;
    image->rows = MATLAB_HDR.SizeY;    
    quantum_info=AcquireQuantumInfo(clone_info,image);
    if (quantum_info == (QuantumInfo *) NULL)
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    image->colors = 1l << image->depth;
    if (image->columns == 0 || image->rows == 0)
      goto MATLAB_KO;

    /* ----- Create gray palette ----- */

    if (CellType==miUINT8 && z!=3)
    {
      if(image->colors>256) image->colors = 256;

      if (!AcquireImageColormap(image, image->colors))
      {
 NoMemory:ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");}
    }

    /*
      If ping is true, then only set image size and colors without
      reading any image data.
    */
    if (image_info->ping)
    {
      unsigned long temp = image->columns;
      image->columns = image->rows;
      image->rows = temp;
      goto done_reading; /* !!!!!! BAD  !!!! */
    }  

  /* ----- Load raster data ----- */
    BImgBuff = (unsigned char *) AcquireQuantumMemory((size_t) (ldblk),sizeof(unsigned char *));    /* Ldblk was set in the check phase */
    if (BImgBuff == NULL)
      goto NoMemory;

    MinVal = 0;
    MaxVal = 0;
    if (CellType==miDOUBLE || CellType==miSINGLE)        /* Find Min and Max Values for floats */
    {
      CalcMinMax(image2, image_info->endian,  MATLAB_HDR.SizeX, MATLAB_HDR.SizeY, CellType, ldblk, BImgBuff, &quantum_info->minimum, &quantum_info->maximum);
    }

    /* Main loop for reading all scanlines */
    if(z==1) z=0; /* read grey scanlines */
		/* else read color scanlines */
    do
    {
      for (i = 0; i < (long) MATLAB_HDR.SizeY; i++)
      {
        q=QueueAuthenticPixels(image,0,MATLAB_HDR.SizeY-i-1,image->columns,1,exception);
        if (q == (PixelPacket *)NULL)
	{
	  if (logging) (void)LogMagickEvent(CoderEvent,GetMagickModule(),
              "  MAT set image pixels returns unexpected NULL on a row %u.", (unsigned)(MATLAB_HDR.SizeY-i-1));
	  goto done_reading;		/* Skip image rotation, when cannot set image pixels	  */
	}
        if(ReadBlob(image2,ldblk,(unsigned char *)BImgBuff) != (ssize_t) ldblk)
	{
	  if (logging) (void)LogMagickEvent(CoderEvent,GetMagickModule(),
             "  MAT cannot read scanrow %u from a file.", (unsigned)(MATLAB_HDR.SizeY-i-1));
	  goto ExitLoop;
	}
        if((CellType==miINT8 || CellType==miUINT8) && (MATLAB_HDR.StructureFlag & FLAG_LOGICAL))
        {
          FixLogical((unsigned char *)BImgBuff,ldblk);
          if(ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,z2qtype[z],BImgBuff,exception) <= 0)
	  {
ImportQuantumPixelsFailed:
	    if (logging) (void)LogMagickEvent(CoderEvent,GetMagickModule(),
              "  MAT failed to ImportQuantumPixels for a row %u", (unsigned)(MATLAB_HDR.SizeY-i-1));
	    break;
	  }
        }
        else
        {
          if(ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,z2qtype[z],BImgBuff,exception) <= 0)
	    goto ImportQuantumPixelsFailed;


          if (z<=1 &&			 /* fix only during a last pass z==0 || z==1 */
	        (CellType==miINT8 || CellType==miINT16 || CellType==miINT32 || CellType==miINT64))
	    FixSignedValues(q,MATLAB_HDR.SizeX);
        }

        if (!SyncAuthenticPixels(image,exception))
	{
	  if (logging) (void)LogMagickEvent(CoderEvent,GetMagickModule(),
            "  MAT failed to sync image pixels for a row %u", (unsigned)(MATLAB_HDR.SizeY-i-1));
	  goto ExitLoop;
	}
      }
    } while(z-- >= 2);
ExitLoop:


    /* Read complex part of numbers here */
    if (MATLAB_HDR.StructureFlag & FLAG_COMPLEX)
    {        /* Find Min and Max Values for complex parts of floats */
      CellType = ReadBlobXXXLong(image2);    /* Additional object type */
      i = ReadBlobXXXLong(image2);           /* size of a complex part - toss away*/

      if (CellType==miDOUBLE || CellType==miSINGLE)
      {
        CalcMinMax(image2,  image_info->endian, MATLAB_HDR.SizeX, MATLAB_HDR.SizeY, CellType, ldblk, BImgBuff, &MinVal, &MaxVal);      
      }

      if (CellType==miDOUBLE)
        for (i = 0; i < (long) MATLAB_HDR.SizeY; i++)
	{
          ReadBlobDoublesXXX(image2, ldblk, (double *)BImgBuff);
          InsertComplexDoubleRow((double *)BImgBuff, i, image, MinVal, MaxVal);
	}

      if (CellType==miSINGLE)
        for (i = 0; i < (long) MATLAB_HDR.SizeY; i++)
	{
          ReadBlobFloatsXXX(image2, ldblk, (float *)BImgBuff);
          InsertComplexFloatRow((float *)BImgBuff, i, image, MinVal, MaxVal);
	}    
    }

      /* Image is gray when no complex flag is set and 2D Matrix AGAIN!!! */
    if ((MATLAB_HDR.DimFlag == 8) &&
        ((MATLAB_HDR.StructureFlag & FLAG_COMPLEX) == 0))
      image->type=GrayscaleType;
    if (image->depth == 1)
      image->type=BilevelType;

    if(image2==image)
        image2 = NULL;    /* Remove shadow copy to an image before rotation. */

      /*  Rotate image. */
    rotated_image = RotateImage(image, 90.0, exception);
    if (rotated_image != (Image *) NULL)
    {
        /* Remove page offsets added by RotateImage */
      rotated_image->page.x=0;
      rotated_image->page.y=0;

      blob = rotated_image->blob;
      rotated_image->blob = image->blob;
      rotated_image->colors = image->colors;
      image->blob = blob;
      AppendImageToList(&image,rotated_image);      
      DeleteImageFromList(&image);      
    }

done_reading:

    if(image2!=NULL)
      if(image2!=image)
      {
        DeleteImageFromList(&image2); 
	if(clone_info)
	{
          if(clone_info->file)
	  {
            fclose(clone_info->file);
            clone_info->file = NULL;
            (void) unlink(clone_info->filename);
	  }
        }    
      }

      /* Allocate next image structure. */    
    AcquireNextImage(image_info,image);
    if (image->next == (Image *) NULL) break;                
    image=SyncNextImageInList(image);
    image->columns=image->rows=0;
    image->colors=0;    

      /* row scan buffer is no longer needed */
    RelinquishMagickMemory(BImgBuff);
    BImgBuff = NULL;
  }
    clone_info=DestroyImageInfo(clone_info);

  RelinquishMagickMemory(BImgBuff);
  CloseBlob(image);


  {
    Image *p;    
    long scene=0;
    
    /*
      Rewind list, removing any empty images while rewinding.
    */
    p=image;
    image=NULL;
    while (p != (Image *)NULL)
      {
        Image *tmp=p;
        if ((p->rows == 0) || (p->columns == 0)) {
          p=p->previous;
          DeleteImageFromList(&tmp);
        } else {
          image=p;
          p=p->previous;
        }
      }
    
    /*
      Fix scene numbers
    */
    for (p=image; p != (Image *) NULL; p=p->next)
      p->scene=scene++;
  }

  if(clone_info != NULL)	/* cleanup garbage file from compression */
  {
    if(clone_info->file)
    {
      fclose(clone_info->file);
      clone_info->file = NULL;
      (void) unlink(clone_info->filename);
    }
    DestroyImageInfo(clone_info);
    clone_info = NULL;
  }
  if (logging) (void)LogMagickEvent(CoderEvent,GetMagickModule(),"return");
  if(image==NULL)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  return (image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r M A T I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method RegisterMATImage adds attributes for the MAT image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterMATImage method is:
%
%      unsigned long RegisterMATImage(void)
%
*/
ModuleExport unsigned long RegisterMATImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("MAT");
  entry->decoder=(DecodeImageHandler *) ReadMATImage;
  entry->encoder=(EncodeImageHandler *) WriteMATImage;
  entry->blob_support=MagickFalse;
  entry->seekable_stream=MagickTrue;
  entry->description=AcquireString("MATLAB image format");
  entry->module=AcquireString("MAT");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r M A T I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method UnregisterMATImage removes format registrations made by the
%  MAT module from the list of supported formats.
%
%  The format of the UnregisterMATImage method is:
%
%      UnregisterMATImage(void)
%
*/
ModuleExport void UnregisterMATImage(void)
{
  (void) UnregisterMagickInfo("MAT");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e M A T L A B I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Function WriteMATImage writes an Matlab matrix to a file.  
%
%  The format of the WriteMATImage method is:
%
%      unsigned int WriteMATImage(const ImageInfo *image_info,Image *image)
%
%  A description of each parameter follows.
%
%    o status: Function WriteMATImage return True if the image is written.
%      False is returned is there is a memory shortage or if the image file
%      fails to write.
%
%    o image_info: Specifies a pointer to a ImageInfo structure.
%
%    o image:  A pointer to an Image structure.
%
*/
static MagickBooleanType WriteMATImage(const ImageInfo *image_info,Image *image)
{
  ExceptionInfo
    *exception;

  long y;
  unsigned z;
  const PixelPacket *p;

  unsigned int status;
  int logging;
  unsigned long DataSize;
  char padding;
  char MATLAB_HDR[0x80];
  time_t current_time;
  struct tm local_time;
  unsigned char *pixels;
  int is_gray;

  MagickOffsetType
    scene;

  QuantumInfo
    *quantum_info;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickSignature);
  logging=LogMagickEvent(CoderEvent,GetMagickModule(),"enter MAT");
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,&image->exception);
  if (status == MagickFalse)
    return(MagickFalse);
  image->depth=8;

  current_time=time((time_t *) NULL);
#if defined(MAGICKCORE_HAVE_LOCALTIME_R)
  (void) localtime_r(&current_time,&local_time);
#else
  (void) memcpy(&local_time,localtime(&current_time),sizeof(local_time));
#endif
  (void) memset(MATLAB_HDR,' ',MagickMin(sizeof(MATLAB_HDR),124));
  FormatMagickString(MATLAB_HDR,MaxTextExtent,"MATLAB 5.0 MAT-file, Platform: %s, Created on: %s %s %2d %2d:%2d:%2d %d",
    OsDesc,DayOfWTab[local_time.tm_wday],MonthsTab[local_time.tm_mon],
    local_time.tm_mday,local_time.tm_hour,local_time.tm_min,
    local_time.tm_sec,local_time.tm_year+1900);
  MATLAB_HDR[0x7C]=0;
  MATLAB_HDR[0x7D]=1;
  MATLAB_HDR[0x7E]='I';
  MATLAB_HDR[0x7F]='M';
  (void) WriteBlob(image,sizeof(MATLAB_HDR),(unsigned char *) MATLAB_HDR);
  scene=0;
  do
  {
    if (image->colorspace != RGBColorspace)
      (void) TransformImageColorspace(image,RGBColorspace);

    is_gray = IsGrayImage(image,&image->exception);
    z = is_gray ? 0 : 3;

    /*
      Store MAT header.
    */
    DataSize = image->rows /*Y*/ * image->columns /*X*/;
    if(!is_gray) DataSize *= 3 /*Z*/;
    padding=((unsigned char)(DataSize-1) & 0x7) ^ 0x7;

    (void) WriteBlobLSBLong(image, miMATRIX);
    (void) WriteBlobLSBLong(image, DataSize+padding+(is_gray ? 48l : 56l));
    (void) WriteBlobLSBLong(image, 0x6); /* 0x88 */
    (void) WriteBlobLSBLong(image, 0x8); /* 0x8C */
    (void) WriteBlobLSBLong(image, 0x6); /* 0x90 */  
    (void) WriteBlobLSBLong(image, 0);   
    (void) WriteBlobLSBLong(image, 0x5); /* 0x98 */
    (void) WriteBlobLSBLong(image, is_gray ? 0x8 : 0xC); /* 0x9C - DimFlag */
    (void) WriteBlobLSBLong(image, image->rows);    /* x: 0xA0 */  
    (void) WriteBlobLSBLong(image, image->columns); /* y: 0xA4 */  
    if(!is_gray)
    {
      (void) WriteBlobLSBLong(image, 3); /* z: 0xA8 */  
      (void) WriteBlobLSBLong(image, 0);
    }
    (void) WriteBlobLSBShort(image, 1);  /* 0xB0 */  
    (void) WriteBlobLSBShort(image, 1);  /* 0xB2 */
    (void) WriteBlobLSBLong(image, 'M'); /* 0xB4 */
    (void) WriteBlobLSBLong(image, 0x2); /* 0xB8 */  
    (void) WriteBlobLSBLong(image, DataSize); /* 0xBC */

    /*
      Store image data.
    */
    exception=(&image->exception);
    quantum_info=AcquireQuantumInfo(image_info,image);
    if (quantum_info == (QuantumInfo *) NULL)
      ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    pixels=GetQuantumPixels(quantum_info);
    do
    {
      for (y=0; y < (long)image->columns; y++)
      {
        p=GetVirtualPixels(image,y,0,1,image->rows,&image->exception);
        if (p == (const PixelPacket *) NULL)
          break;
        (void) ExportQuantumPixels(image,(const CacheView *) NULL,quantum_info,
          z2qtype[z],pixels,exception);
        (void) WriteBlob(image,image->rows,pixels);
      }    
      if (!SyncAuthenticPixels(image,exception))
        break;
    } while(z-- >= 2);
    while(padding-->0) (void) WriteBlobByte(image,0);
    quantum_info=DestroyQuantumInfo(quantum_info);
    if (GetNextImageInList(image) == (Image *) NULL)
      break;
    image=SyncNextImageInList(image);
    status=SetImageProgress(image,SaveImagesTag,scene++,
      GetImageListLength(image));
    if (status == MagickFalse)
      break;
  } while (image_info->adjoin != MagickFalse);
  (void) CloseBlob(image);
  return(MagickTrue);
}
