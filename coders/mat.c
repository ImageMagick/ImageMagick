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
#include "MagickCore/studio.h"
#include "MagickCore/attribute.h"
#include "MagickCore/blob.h"
#include "MagickCore/blob-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colormap.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/distort.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/quantum.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/option.h"
#include "MagickCore/pixel.h"
#include "MagickCore/resource_.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"
#include "MagickCore/transform.h"
#include "MagickCore/utility-private.h"
#if defined(MAGICKCORE_ZLIB_DELEGATE)
 #include "zlib.h"
#endif

/*
  Forward declaration.
*/
static MagickBooleanType
  WriteMATImage(const ImageInfo *,Image *,ExceptionInfo *);


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
#if defined(MAGICKCORE_WINDOWS_SUPPORT)
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
    miINT8 = 1,      /* 8 bit signed */
    miUINT8,      /* 8 bit unsigned */
    miINT16,      /* 16 bit signed */
    miUINT16,      /* 16 bit unsigned */
    miINT32,      /* 32 bit signed */
    miUINT32,      /* 32 bit unsigned */
    miSINGLE,      /* IEEE 754 single precision float */
    miRESERVE1,
    miDOUBLE,      /* IEEE 754 double precision float */
    miRESERVE2,
    miRESERVE3,
    miINT64,      /* 64 bit signed */
    miUINT64,      /* 64 bit unsigned */
    miMATRIX,            /* MATLAB array */
    miCOMPRESSED,          /* Compressed Data */
    miUTF8,            /* Unicode UTF-8 Encoded Character Data */
    miUTF16,            /* Unicode UTF-16 Encoded Character Data */
    miUTF32      /* Unicode UTF-32 Encoded Character Data */
  } mat5_data_type;

typedef enum
  {
    mxCELL_CLASS=1,    /* cell array */
    mxSTRUCT_CLASS,    /* structure */
    mxOBJECT_CLASS,    /* object */
    mxCHAR_CLASS,    /* character array */
    mxSPARSE_CLASS,    /* sparse array */
    mxDOUBLE_CLASS,    /* double precision array */
    mxSINGLE_CLASS,    /* single precision floating point */
    mxINT8_CLASS,    /* 8 bit signed integer */
    mxUINT8_CLASS,    /* 8 bit unsigned integer */
    mxINT16_CLASS,    /* 16 bit signed integer */
    mxUINT16_CLASS,    /* 16 bit unsigned integer */
    mxINT32_CLASS,    /* 32 bit signed integer */
    mxUINT32_CLASS,    /* 32 bit unsigned integer */
    mxINT64_CLASS,    /* 64 bit signed integer */
    mxUINT64_CLASS,    /* 64 bit unsigned integer */
    mxFUNCTION_CLASS            /* Function handle */
  } arrayclasstype;

#define FLAG_COMPLEX 0x8
#define FLAG_GLOBAL  0x4
#define FLAG_LOGICAL 0x2

static const QuantumType z2qtype[4] = {GrayQuantum, BlueQuantum, GreenQuantum, RedQuantum};


static void InsertComplexDoubleRow(Image *image,double *p,int y,double MinVal,
  double MaxVal,ExceptionInfo *exception)
{

  double f;
  int x;
  register Quantum *q;

  if (MinVal == 0)
    MinVal = -1;
  if (MaxVal == 0)
    MaxVal = 1;

  q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
  if (q == (Quantum *) NULL)
    return;
  for (x = 0; x < (ssize_t) image->columns; x++)
  {
    if (*p > 0)
    {
      f = (*p / MaxVal) * (QuantumRange-GetPixelRed(image,q));
      if (f + GetPixelRed(image,q) > QuantumRange)
        SetPixelRed(image,QuantumRange,q);
      else
        SetPixelRed(image,GetPixelRed(image,q)+(int) f,q);
      if ((int) f / 2.0 > GetPixelGreen(image,q))
        {
          SetPixelGreen(image,0,q);
          SetPixelBlue(image,0,q);
        }
      else
        {
          SetPixelBlue(image,GetPixelBlue(image,q)-(int) (f/2.0),q);
          SetPixelGreen(image,GetPixelBlue(image,q),q);
        }
    }
    if (*p < 0)
    {
      f = (*p / MaxVal) * (QuantumRange-GetPixelBlue(image,q));
      if (f+GetPixelBlue(image,q) > QuantumRange)
        SetPixelBlue(image,QuantumRange,q);
      else
        SetPixelBlue(image,GetPixelBlue(image,q)+(int) f,q);
      if ((int) f / 2.0 > GetPixelGreen(image,q))
        {
          SetPixelRed(image,0,q);
          SetPixelGreen(image,0,q);
        }
      else
        {
          SetPixelRed(image,GetPixelRed(image,q)-(int) (f/2.0),q);
          SetPixelGreen(image,GetPixelRed(image,q),q);
        }
    }
    p++;
    q+=GetPixelChannels(image);
  }
  if (!SyncAuthenticPixels(image,exception))
    return;
  return;
}


static void InsertComplexFloatRow(Image *image,float *p,int y,double MinVal,
  double MaxVal,ExceptionInfo *exception)
{
  double f;
  int x;
  register Quantum *q;

  if (MinVal == 0)
    MinVal = -1;
  if (MaxVal == 0)
    MaxVal = 1;

  q = QueueAuthenticPixels(image, 0, y, image->columns, 1,exception);
  if (q == (Quantum *) NULL)
    return;
  for (x = 0; x < (ssize_t) image->columns; x++)
  {
    if (*p > 0)
    {
      f = (*p / MaxVal) * (QuantumRange-GetPixelRed(image,q));
      if (f+GetPixelRed(image,q) > QuantumRange)
        SetPixelRed(image,QuantumRange,q);
      else
        SetPixelRed(image,GetPixelRed(image,q)+(int) f,q);
      if ((int) f / 2.0 > GetPixelGreen(image,q))
        {
          SetPixelGreen(image,0,q);
          SetPixelBlue(image,0,q);
        }
      else
        {
          SetPixelBlue(image,GetPixelBlue(image,q)-(int) (f/2.0),q);
          SetPixelGreen(image,GetPixelBlue(image,q),q);
        }
    }
    if (*p < 0)
    {
      f = (*p / MaxVal) * (QuantumRange - GetPixelBlue(image,q));
      if (f + GetPixelBlue(image,q) > QuantumRange)
        SetPixelBlue(image,QuantumRange,q);
      else
        SetPixelBlue(image,GetPixelBlue(image,q)+
          (int) f,q);
      if ((int) f / 2.0 > GetPixelGreen(image,q))
        {
          SetPixelGreen(image,0,q);
          SetPixelRed(image,0,q);
        }
      else
        {
          SetPixelRed(image,GetPixelRed(image,q)-(int) (f/2.0),q);
          SetPixelGreen(image,GetPixelRed(image,q),q);
        }
    }
    p++;
    q++;
  }
  if (!SyncAuthenticPixels(image,exception))
    return;
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
static void CalcMinMax(Image *image, int endian_indicator, int SizeX, int SizeY, size_t CellType, unsigned ldblk, void *BImgBuff, double *Min, double *Max)
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
  else    /* MI */
  {
    ReadBlobDoublesXXX = ReadBlobDoublesMSB;
    ReadBlobFloatsXXX = ReadBlobFloatsMSB;
  }

  filepos = TellBlob(image);     /* Please note that file seeking occurs only in the case of doubles */
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
    for (x = 0; x < (ssize_t) SizeX; x++)
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


static void FixSignedValues(const Image *image,Quantum *q, int y)
{
  while(y-->0)
  {
     /* Please note that negative values will overflow
        Q=8; QuantumRange=255: <0;127> + 127+1 = <128; 255>
           <-1;-128> + 127+1 = <0; 127> */
    SetPixelRed(image,GetPixelRed(image,q)+QuantumRange/2+1,q);
    SetPixelGreen(image,GetPixelGreen(image,q)+QuantumRange/2+1,q);
    SetPixelBlue(image,GetPixelBlue(image,q)+QuantumRange/2+1,q);
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

#if defined(MAGICKCORE_ZLIB_DELEGATE)
/** This procedure decompreses an image block for a new MATLAB format. */
static Image *DecompressBlock(Image *orig, MagickOffsetType Size, ImageInfo *clone_info, ExceptionInfo *exception)
{

Image *image2;
void *CacheBlock, *DecompressBlock;
z_stream zip_info;
FILE *mat_file;
size_t magick_size;
size_t extent;
int file;

int status;
int zip_status;

  if(clone_info==NULL) return NULL;
  if(clone_info->file)    /* Close file opened from previous transaction. */
  {
    fclose(clone_info->file);
    clone_info->file = NULL;
    (void) remove_utf8(clone_info->filename);
  }

  CacheBlock = AcquireQuantumMemory((size_t)((Size<16384)?Size:16384),sizeof(unsigned char *));
  if(CacheBlock==NULL) return NULL;
  DecompressBlock = AcquireQuantumMemory((size_t)(4096),sizeof(unsigned char *));
  if(DecompressBlock==NULL)
  {
    RelinquishMagickMemory(CacheBlock);
    return NULL;
  }

  mat_file=0;
  file = AcquireUniqueFileResource(clone_info->filename);
  if (file != -1)
    mat_file = fdopen(file,"w");
  if(!mat_file)
  {
    RelinquishMagickMemory(CacheBlock);
    RelinquishMagickMemory(DecompressBlock);
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),"Cannot create file stream for decompressed image");
    return NULL;
  }

  zip_info.zalloc=AcquireZIPMemory;
  zip_info.zfree=RelinquishZIPMemory;
  zip_info.opaque = (voidpf) NULL;
  zip_status = inflateInit(&zip_info);
  if (zip_status != Z_OK)
    {
      RelinquishMagickMemory(CacheBlock);
      RelinquishMagickMemory(DecompressBlock);
      (void) ThrowMagickException(exception,GetMagickModule(),CorruptImageError,
        "UnableToUncompressImage","`%s'",clone_info->filename);
      (void) fclose(mat_file);
      RelinquishUniqueFileResource(clone_info->filename);
      return NULL;
    }
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
      zip_status = inflate(&zip_info,Z_NO_FLUSH);
      if ((zip_status != Z_OK) && (zip_status != Z_STREAM_END))
        break;
      extent=fwrite(DecompressBlock, 4096-zip_info.avail_out, 1, mat_file);
      (void) extent;

      if(zip_status == Z_STREAM_END) goto DblBreak;
    }
    if ((zip_status != Z_OK) && (zip_status != Z_STREAM_END))
      break;

    Size -= magick_size;
  }
DblBreak:

  inflateEnd(&zip_info);
  (void)fclose(mat_file);
  RelinquishMagickMemory(CacheBlock);
  RelinquishMagickMemory(DecompressBlock);

  if((clone_info->file=fopen(clone_info->filename,"rb"))==NULL) goto UnlinkFile;
  if( (image2 = AcquireImage(clone_info,exception))==NULL ) goto EraseFile;
  status = OpenBlob(clone_info,image2,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
  {
    DeleteImageFromList(&image2);
EraseFile:
    fclose(clone_info->file);
    clone_info->file = NULL;
UnlinkFile:
    RelinquishUniqueFileResource(clone_info->filename);
    return NULL;
  }

  return image2;
}
#endif

static Image *ReadMATImageV4(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  typedef struct {
    unsigned char Type[4];
    unsigned int nRows;
    unsigned int nCols;
    unsigned int imagf;
    unsigned int nameLen;
  } MAT4_HDR;

  long
    ldblk;

  EndianType
    endian;

  Image
    *rotate_image;

  MagickBooleanType
    status;

  MAT4_HDR
    HDR;

  QuantumInfo
    *quantum_info;

  QuantumFormatType
    format_type;

  register ssize_t
    i;

  ssize_t
    count,
    y;

  unsigned char
    *pixels;

  unsigned int
    depth;


  (void) SeekBlob(image,0,SEEK_SET);
  while (EOFBlob(image) != MagickFalse)
  {
    /*
     Object parser loop.
    */
    ldblk=ReadBlobLSBLong(image);
    if ((ldblk > 9999) || (ldblk < 0))
      break;
    HDR.Type[3]=ldblk % 10; ldblk /= 10;  /* T digit */
    HDR.Type[2]=ldblk % 10; ldblk /= 10;  /* P digit */
    HDR.Type[1]=ldblk % 10; ldblk /= 10;  /* O digit */
    HDR.Type[0]=ldblk;        /* M digit */
    if (HDR.Type[3] != 0)
      break;  /* Data format */
    if (HDR.Type[2] != 0)
      break;  /* Always 0 */
    if (HDR.Type[0] == 0)
      {
        HDR.nRows=ReadBlobLSBLong(image);
        HDR.nCols=ReadBlobLSBLong(image);
        HDR.imagf=ReadBlobLSBLong(image);
        HDR.nameLen=ReadBlobLSBLong(image);
        endian=LSBEndian;
      }
    else
      {
        HDR.nRows=ReadBlobMSBLong(image);
        HDR.nCols=ReadBlobMSBLong(image);
        HDR.imagf=ReadBlobMSBLong(image);
        HDR.nameLen=ReadBlobMSBLong(image);
        endian=MSBEndian;
      }
    if ((HDR.imagf != 0) && (HDR.imagf != 1))
      break;
    if (HDR.nameLen > 0xFFFF)
      return((Image *) NULL);
    for (i=0; i < (ssize_t) HDR.nameLen; i++)
    {
      int
        byte;

      /*
        Skip matrix name.
      */
      byte=ReadBlobByte(image);
      if (byte == EOF)
        {
          ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
            image->filename);
          break;
        }
    }
    image->columns=(size_t) HDR.nRows;
    image->rows=(size_t) HDR.nCols;
    SetImageColorspace(image,GRAYColorspace,exception);
    if (image_info->ping != MagickFalse)
      {
        Swap(image->columns,image->rows);
        return(image);
      }
    status=SetImageExtent(image,image->columns,image->rows,exception);
    if (status == MagickFalse)
      return((Image *) NULL);
    quantum_info=AcquireQuantumInfo(image_info,image);
    if (quantum_info == (QuantumInfo *) NULL)
      return((Image *) NULL);
    switch(HDR.Type[1])
    {
      case 0:
        format_type=FloatingPointQuantumFormat;
        depth=64;
        break;
      case 1:
        format_type=FloatingPointQuantumFormat;
        depth=32;
        break;
      case 2:
        format_type=UnsignedQuantumFormat;
        depth=16;
        break;
      case 3:
        format_type=SignedQuantumFormat;
        depth=16;
        break;
      case 4:
        format_type=UnsignedQuantumFormat;
        depth=8;
        break;
      default:
        format_type=UnsignedQuantumFormat;
        depth=8;
        break;
    }
    image->depth=depth;
    if (HDR.Type[0] != 0)
      SetQuantumEndian(image,quantum_info,MSBEndian);
    status=SetQuantumFormat(image,quantum_info,format_type);
    status=SetQuantumDepth(image,quantum_info,depth);
    status=SetQuantumEndian(image,quantum_info,endian);
    SetQuantumScale(quantum_info,1.0);
    pixels=(unsigned char *) GetQuantumPixels(quantum_info);
    for (y=0; y < (ssize_t) image->rows; y++)
    {
      int
        status;

      register Quantum
        *magick_restrict q;

      count=ReadBlob(image,depth/8*image->columns,(char *) pixels);
      if (count == -1)
        break;
      q=QueueAuthenticPixels(image,0,image->rows-y-1,image->columns,1,
        exception);
      if (q == (Quantum *) NULL)
        break;
      (void) ImportQuantumPixels(image,(CacheView *) NULL,quantum_info,
        GrayQuantum,pixels,exception);
      if ((HDR.Type[1] == 2) || (HDR.Type[1] == 3))
        FixSignedValues(image,q,(int) image->columns);
      if (SyncAuthenticPixels(image,exception) == MagickFalse)
        break;
      if (image->previous == (Image *) NULL)
        {
          status=SetImageProgress(image,LoadImageTag,(MagickOffsetType) y,
            image->rows);
          if (status == MagickFalse)
            break;
        }
    }
    if (HDR.imagf == 1)
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        /*
          Read complex pixels.
        */
        count=ReadBlob(image,depth/8*image->columns,(char *) pixels);
        if (count == -1)
          break;
        if (HDR.Type[1] == 0)
          InsertComplexDoubleRow(image,(double *) pixels,y,0,0,exception);
        else
          InsertComplexFloatRow(image,(float *) pixels,y,0,0,exception);
      }
    quantum_info=DestroyQuantumInfo(quantum_info);
    rotate_image=RotateImage(image,90.0,exception);
    if (rotate_image != (Image *) NULL)
      {
        image=DestroyImage(image);
        image=rotate_image;
      }
    if (EOFBlob(image) != MagickFalse)
      {
        ThrowFileException(exception,CorruptImageError,"UnexpectedEndOfFile",
          image->filename);
        break;
      }
    /*
      Proceed to next image.
    */
    if (image_info->number_scenes != 0)
      if (image->scene >= (image_info->scene+image_info->number_scenes-1))
        break;
    /*
      Allocate next image structure.
    */
    AcquireNextImage(image_info,image,exception);
    if (GetNextImageInList(image) == (Image *) NULL)
      {
        image=DestroyImageList(image);
        return((Image *) NULL);
      }
    image=SyncNextImageInList(image);
    status=SetImageProgress(image,LoadImagesTag,TellBlob(image),
      GetBlobSize(image));
    if (status == MagickFalse)
      break;
  }
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
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
static Image *ReadMATImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  Image *image, *image2=NULL,
   *rotated_image;
  register Quantum *q;

  unsigned int status;
  MATHeader MATLAB_HDR;
  size_t size;
  size_t CellType;
  QuantumInfo *quantum_info;
  ImageInfo *clone_info;
  int i;
  ssize_t ldblk;
  unsigned char *BImgBuff = NULL;
  double MinVal, MaxVal;
  unsigned z, z2;
  unsigned Frames;
  int logging;
  int sample_size;
  MagickOffsetType filepos=0x80;
  BlobInfo *blob;
  size_t one;

  unsigned int (*ReadBlobXXXLong)(Image *image);
  unsigned short (*ReadBlobXXXShort)(Image *image);
  void (*ReadBlobDoublesXXX)(Image * image, size_t len, double *data);
  void (*ReadBlobFloatsXXX)(Image * image, size_t len, float *data);


  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  logging = LogMagickEvent(CoderEvent,GetMagickModule(),"enter");

  /*
     Open image file.
   */
  image = AcquireImage(image_info,exception);

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
  if (ReadBlob(image,124,(unsigned char *) &MATLAB_HDR.identific) != 124)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  if (strncmp(MATLAB_HDR.identific,"MATLAB",6) != 0)
    {
      image2=ReadMATImageV4(image_info,image,exception);
      if (image2  == NULL)
        goto MATLAB_KO;
      image=image2;
      goto END_OF_READING;
    }
  MATLAB_HDR.Version = ReadBlobLSBShort(image);
  if(ReadBlob(image,2,(unsigned char *) &MATLAB_HDR.EndianIndicator) != 2)
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");

  if (logging)
    (void) LogMagickEvent(CoderEvent,GetMagickModule(),"  Endian %c%c",
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
    Frames = 1;
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
      MATLAB_HDR.unknown4 = ReadBlobXXXLong(image2);  /* ??? don't understand why ?? */
    MATLAB_HDR.unknown4 = ReadBlobXXXLong(image2);
    MATLAB_HDR.DimFlag = ReadBlobXXXLong(image2);
    MATLAB_HDR.SizeX = ReadBlobXXXLong(image2);
    MATLAB_HDR.SizeY = ReadBlobXXXLong(image2);


    switch(MATLAB_HDR.DimFlag)
    {
      case  8: z2=z=1; break;      /* 2D matrix*/
      case 12: z2=z = ReadBlobXXXLong(image2);  /* 3D matrix RGB*/
           (void) ReadBlobXXXLong(image2);
         if(z!=3) ThrowReaderException(CoderError, "MultidimensionalMatricesAreNotSupported");
         break;
      case 16: z2=z = ReadBlobXXXLong(image2);  /* 4D matrix animation */
         if(z!=3 && z!=1)
            ThrowReaderException(CoderError, "MultidimensionalMatricesAreNotSupported");
         Frames = ReadBlobXXXLong(image2);
         if (Frames == 0)
           ThrowReaderException(CorruptImageError,"ImproperImageHeader");
         break;
      default: ThrowReaderException(CoderError, "MultidimensionalMatricesAreNotSupported");
    }

    MATLAB_HDR.Flag1 = ReadBlobXXXShort(image2);
    MATLAB_HDR.NameFlag = ReadBlobXXXShort(image2);

    if (logging) (void)LogMagickEvent(CoderEvent,GetMagickModule(),
          "MATLAB_HDR.StructureClass %d",MATLAB_HDR.StructureClass);
    if (MATLAB_HDR.StructureClass != mxCHAR_CLASS &&
        MATLAB_HDR.StructureClass != mxSINGLE_CLASS &&    /* float + complex float */
        MATLAB_HDR.StructureClass != mxDOUBLE_CLASS &&    /* double + complex double */
        MATLAB_HDR.StructureClass != mxINT8_CLASS &&
        MATLAB_HDR.StructureClass != mxUINT8_CLASS &&    /* uint8 + uint8 3D */
        MATLAB_HDR.StructureClass != mxINT16_CLASS &&
        MATLAB_HDR.StructureClass != mxUINT16_CLASS &&    /* uint16 + uint16 3D */
        MATLAB_HDR.StructureClass != mxINT32_CLASS &&
        MATLAB_HDR.StructureClass != mxUINT32_CLASS &&    /* uint32 + uint32 3D */
        MATLAB_HDR.StructureClass != mxINT64_CLASS &&
        MATLAB_HDR.StructureClass != mxUINT64_CLASS)    /* uint64 + uint64 3D */
      ThrowReaderException(CoderError,"UnsupportedCellTypeInTheMatrix");

    switch (MATLAB_HDR.NameFlag)
    {
      case 0:
        size = ReadBlobXXXLong(image2);  /* Object name string size */
        size = 4 * (ssize_t) ((size + 3 + 1) / 4);
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
    if (logging)
      (void) LogMagickEvent(CoderEvent,GetMagickModule(),
        "MATLAB_HDR.CellType: %.20g",(double) CellType);

    (void) ReadBlob(image2, 4, (unsigned char *) &size);     /* data size */

    NEXT_FRAME:
    switch (CellType)
    {
      case miINT8:
      case miUINT8:
        sample_size = 8;
        if(MATLAB_HDR.StructureFlag & FLAG_LOGICAL)
          image->depth = 1;
        else
          image->depth = 8;         /* Byte type cell */
        ldblk = (ssize_t) MATLAB_HDR.SizeX;
        break;
      case miINT16:
      case miUINT16:
        sample_size = 16;
        image->depth = 16;        /* Word type cell */
        ldblk = (ssize_t) (2 * MATLAB_HDR.SizeX);
        break;
      case miINT32:
      case miUINT32:
        sample_size = 32;
        image->depth = 32;        /* Dword type cell */
        ldblk = (ssize_t) (4 * MATLAB_HDR.SizeX);
        break;
      case miINT64:
      case miUINT64:
        sample_size = 64;
        image->depth = 64;        /* Qword type cell */
        ldblk = (ssize_t) (8 * MATLAB_HDR.SizeX);
        break;
      case miSINGLE:
        sample_size = 32;
        image->depth = 32;        /* double type cell */
        (void) SetImageOption(clone_info,"quantum:format","floating-point");
        if (MATLAB_HDR.StructureFlag & FLAG_COMPLEX)
  {              /* complex float type cell */
  }
        ldblk = (ssize_t) (4 * MATLAB_HDR.SizeX);
        break;
      case miDOUBLE:
        sample_size = 64;
        image->depth = 64;        /* double type cell */
        (void) SetImageOption(clone_info,"quantum:format","floating-point");
DisableMSCWarning(4127)
        if (sizeof(double) != 8)
RestoreMSCWarning
          ThrowReaderException(CoderError, "IncompatibleSizeOfDouble");
        if (MATLAB_HDR.StructureFlag & FLAG_COMPLEX)
  {                         /* complex double type cell */
  }
        ldblk = (ssize_t) (8 * MATLAB_HDR.SizeX);
        break;
      default:
        ThrowReaderException(CoderError, "UnsupportedCellTypeInTheMatrix");
    }
    (void) sample_size;
    image->columns = MATLAB_HDR.SizeX;
    image->rows = MATLAB_HDR.SizeY;
    quantum_info=AcquireQuantumInfo(clone_info,image);
    if (quantum_info == (QuantumInfo *) NULL)
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    one=1;
    image->colors = one << image->depth;
    if (image->columns == 0 || image->rows == 0)
      goto MATLAB_KO;
    /* Image is gray when no complex flag is set and 2D Matrix */
    if ((MATLAB_HDR.DimFlag == 8) &&
        ((MATLAB_HDR.StructureFlag & FLAG_COMPLEX) == 0))
      {
        image->type=GrayscaleType;
        SetImageColorspace(image,GRAYColorspace,exception);
      }


    /*
      If ping is true, then only set image size and colors without
      reading any image data.
    */
    if (image_info->ping)
    {
      size_t temp = image->columns;
      image->columns = image->rows;
      image->rows = temp;
      goto done_reading; /* !!!!!! BAD  !!!! */
    }
    status=SetImageExtent(image,image->columns,image->rows,exception);
    if (status == MagickFalse)
      return(DestroyImageList(image));

  /* ----- Load raster data ----- */
    BImgBuff = (unsigned char *) AcquireQuantumMemory((size_t) (ldblk),sizeof(double));    /* Ldblk was set in the check phase */
    if (BImgBuff == NULL)
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    (void) ResetMagickMemory(BImgBuff,0,ldblk*sizeof(double));

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
      for (i = 0; i < (ssize_t) MATLAB_HDR.SizeY; i++)
      {
        q=GetAuthenticPixels(image,0,MATLAB_HDR.SizeY-i-1,image->columns,1,exception);
        if (q == (Quantum *) NULL)
  {
    if (logging) (void)LogMagickEvent(CoderEvent,GetMagickModule(),
              "  MAT set image pixels returns unexpected NULL on a row %u.", (unsigned)(MATLAB_HDR.SizeY-i-1));
    goto done_reading;    /* Skip image rotation, when cannot set image pixels    */
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


          if (z<=1 &&       /* fix only during a last pass z==0 || z==1 */
          (CellType==miINT8 || CellType==miINT16 || CellType==miINT32 || CellType==miINT64))
      FixSignedValues(image,q,MATLAB_HDR.SizeX);
        }

        if (!SyncAuthenticPixels(image,exception))
  {
    if (logging) (void)LogMagickEvent(CoderEvent,GetMagickModule(),
            "  MAT failed to sync image pixels for a row %u", (unsigned)(MATLAB_HDR.SizeY-i-1));
    goto ExitLoop;
  }
      }
    } while(z-- >= 2);
    quantum_info=DestroyQuantumInfo(quantum_info);
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
        for (i = 0; i < (ssize_t) MATLAB_HDR.SizeY; i++)
  {
          ReadBlobDoublesXXX(image2, ldblk, (double *)BImgBuff);
          InsertComplexDoubleRow(image, (double *)BImgBuff, i, MinVal, MaxVal,
            exception);
  }

      if (CellType==miSINGLE)
        for (i = 0; i < (ssize_t) MATLAB_HDR.SizeY; i++)
  {
          ReadBlobFloatsXXX(image2, ldblk, (float *)BImgBuff);
          InsertComplexFloatRow(image,(float *)BImgBuff,i,MinVal,MaxVal,
            exception);
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
            (void) remove_utf8(clone_info->filename);
    }
        }
      }

      /* Allocate next image structure. */
    AcquireNextImage(image_info,image,exception);
    if (image->next == (Image *) NULL) break;
    image=SyncNextImageInList(image);
    image->columns=image->rows=0;
    image->colors=0;

      /* row scan buffer is no longer needed */
    RelinquishMagickMemory(BImgBuff);
    BImgBuff = NULL;

    if(--Frames>0)
    {
      z = z2;
      if(image2==NULL) image2 = image;
      goto NEXT_FRAME;
    }
    if ((image2!=NULL) && (image2!=image))   /* Does shadow temporary decompressed image exist? */
      {
/*  CloseBlob(image2); */
        DeleteImageFromList(&image2);
        if(clone_info)
        {
          if(clone_info->file)
          {
            fclose(clone_info->file);
            clone_info->file = NULL;
            (void) remove_utf8(clone_info->filename);
          }
        }
        }
  }

  RelinquishMagickMemory(BImgBuff);
END_OF_READING:
  clone_info=DestroyImageInfo(clone_info);
  CloseBlob(image);


  {
    Image *p;
    ssize_t scene=0;

    /*
      Rewind list, removing any empty images while rewinding.
    */
    p=image;
    image=NULL;
    while (p != (Image *) NULL)
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

  if(clone_info != NULL)  /* cleanup garbage file from compression */
  {
    if(clone_info->file)
    {
      fclose(clone_info->file);
      clone_info->file = NULL;
      (void) remove_utf8(clone_info->filename);
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
%      size_t RegisterMATImage(void)
%
*/
ModuleExport size_t RegisterMATImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("MAT","MAT","MATLAB level 5 image format");
  entry->decoder=(DecodeImageHandler *) ReadMATImage;
  entry->encoder=(EncodeImageHandler *) WriteMATImage;
  entry->flags^=CoderBlobSupportFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
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
%      MagickBooleanType WriteMATImage(const ImageInfo *image_info,
%        Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image_info: Specifies a pointer to a ImageInfo structure.
%
%    o image:  A pointer to an Image structure.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static MagickBooleanType WriteMATImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  ssize_t y;
  unsigned z;
  register const Quantum *p;

  unsigned int status;
  int logging;
  size_t DataSize;
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
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  logging=LogMagickEvent(CoderEvent,GetMagickModule(),"enter MAT");
  (void) logging;
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
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
  FormatLocaleString(MATLAB_HDR,sizeof(MATLAB_HDR),
    "MATLAB 5.0 MAT-file, Platform: %s, Created on: %s %s %2d %2d:%2d:%2d %d",
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
    (void) TransformImageColorspace(image,sRGBColorspace,exception);
    is_gray = SetImageGray(image,exception);
    z = is_gray ? 0 : 3;

    /*
      Store MAT header.
    */
    DataSize = image->rows /*Y*/ * image->columns /*X*/;
    if(!is_gray) DataSize *= 3 /*Z*/;
    padding=((unsigned char)(DataSize-1) & 0x7) ^ 0x7;

    (void) WriteBlobLSBLong(image, miMATRIX);
    (void) WriteBlobLSBLong(image, (unsigned int) DataSize+padding+(is_gray ? 48 : 56));
    (void) WriteBlobLSBLong(image, 0x6); /* 0x88 */
    (void) WriteBlobLSBLong(image, 0x8); /* 0x8C */
    (void) WriteBlobLSBLong(image, 0x6); /* 0x90 */
    (void) WriteBlobLSBLong(image, 0);
    (void) WriteBlobLSBLong(image, 0x5); /* 0x98 */
    (void) WriteBlobLSBLong(image, is_gray ? 0x8 : 0xC); /* 0x9C - DimFlag */
    (void) WriteBlobLSBLong(image, (unsigned int) image->rows);    /* x: 0xA0 */
    (void) WriteBlobLSBLong(image, (unsigned int) image->columns); /* y: 0xA4 */
    if(!is_gray)
    {
      (void) WriteBlobLSBLong(image, 3); /* z: 0xA8 */
      (void) WriteBlobLSBLong(image, 0);
    }
    (void) WriteBlobLSBShort(image, 1);  /* 0xB0 */
    (void) WriteBlobLSBShort(image, 1);  /* 0xB2 */
    (void) WriteBlobLSBLong(image, 'M'); /* 0xB4 */
    (void) WriteBlobLSBLong(image, 0x2); /* 0xB8 */
    (void) WriteBlobLSBLong(image, (unsigned int) DataSize); /* 0xBC */

    /*
      Store image data.
    */
    quantum_info=AcquireQuantumInfo(image_info,image);
    if (quantum_info == (QuantumInfo *) NULL)
      ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    pixels=(unsigned char *) GetQuantumPixels(quantum_info);
    do
    {
      for (y=0; y < (ssize_t)image->columns; y++)
      {
        p=GetVirtualPixels(image,y,0,1,image->rows,exception);
        if (p == (const Quantum *) NULL)
          break;
        (void) ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
          z2qtype[z],pixels,exception);
        (void) WriteBlob(image,image->rows,pixels);
      }
      if (SyncAuthenticPixels(image,exception) == MagickFalse)
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
