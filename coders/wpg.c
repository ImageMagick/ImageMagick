/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                            W   W  PPPP    GGGG                              %
%                            W   W  P   P  G                                  %
%                            W W W  PPPP   G GGG                              %
%                            WW WW  P      G   G                              %
%                            W   W  P       GGG                               %
%                                                                             %
%                                                                             %
%                       Read WordPerfect Image Format                         %
%                                                                             %
%                              Software Design                                %
%                              Jaroslav Fojtik                                %
%                                 June 2000                                   %
%                                                                             %
%                                                                             %
%  Copyright @ 1999 ImageMagick Studio LLC, a non-profit organization         %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    https://imagemagick.org/script/license.php                               %
%                                                                             %
%  Unless required by applicable law or agreed to in writing, software        %
%  distributed under the License is distributed on an "AS IS" BASIS,          %
%  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.   %
%  See the License for the specific language governing permissions and        %
%  limitations under the License.                                             %
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
#include "MagickCore/color-private.h"
#include "MagickCore/colormap.h"
#include "MagickCore/colormap-private.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/constitute.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/cache.h"
#include "MagickCore/distort.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/magic.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/resource_.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/module.h"
#include "MagickCore/transform.h"
#include "MagickCore/utility.h"
#include "MagickCore/utility-private.h"

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteWPGImage(const ImageInfo *,Image *,ExceptionInfo *);

typedef struct
{
  unsigned char Red;
  unsigned char Blue;
  unsigned char Green;
} RGB_Record;

/* Default palette for WPG level 1 */
static const RGB_Record WPG1_Palette[256]=
{
  {  0,  0,  0},    {  0,  0,168},
  {  0,168,  0},    {  0,168,168},
  {168,  0,  0},    {168,  0,168},
  {168, 84,  0},    {168,168,168},
  { 84, 84, 84},    { 84, 84,252},
  { 84,252, 84},    { 84,252,252},
  {252, 84, 84},    {252, 84,252},
  {252,252, 84},    {252,252,252},  /*16*/
  {  0,  0,  0},    { 20, 20, 20},
  { 32, 32, 32},    { 44, 44, 44},
  { 56, 56, 56},    { 68, 68, 68},
  { 80, 80, 80},    { 96, 96, 96},
  {112,112,112},    {128,128,128},
  {144,144,144},    {160,160,160},
  {180,180,180},    {200,200,200},
  {224,224,224},    {252,252,252},  /*32*/
  {  0,  0,252},    { 64,  0,252},
  {124,  0,252},    {188,  0,252},
  {252,  0,252},    {252,  0,188},
  {252,  0,124},    {252,  0, 64},
  {252,  0,  0},    {252, 64,  0},
  {252,124,  0},    {252,188,  0},
  {252,252,  0},    {188,252,  0},
  {124,252,  0},    { 64,252,  0},  /*48*/
  {  0,252,  0},    {  0,252, 64},
  {  0,252,124},    {  0,252,188},
  {  0,252,252},    {  0,188,252},
  {  0,124,252},    {  0, 64,252},
  {124,124,252},    {156,124,252},
  {188,124,252},    {220,124,252},
  {252,124,252},    {252,124,220},
  {252,124,188},    {252,124,156},  /*64*/
  {252,124,124},    {252,156,124},
  {252,188,124},    {252,220,124},
  {252,252,124},    {220,252,124},
  {188,252,124},    {156,252,124},
  {124,252,124},    {124,252,156},
  {124,252,188},    {124,252,220},
  {124,252,252},    {124,220,252},
  {124,188,252},    {124,156,252},  /*80*/
  {180,180,252},    {196,180,252},
  {216,180,252},    {232,180,252},
  {252,180,252},    {252,180,232},
  {252,180,216},    {252,180,196},
  {252,180,180},    {252,196,180},
  {252,216,180},    {252,232,180},
  {252,252,180},    {232,252,180},
  {216,252,180},    {196,252,180},  /*96*/
  {180,220,180},    {180,252,196},
  {180,252,216},    {180,252,232},
  {180,252,252},    {180,232,252},
  {180,216,252},    {180,196,252},
  {0,0,112},    {28,0,112},
  {56,0,112},    {84,0,112},
  {112,0,112},    {112,0,84},
  {112,0,56},    {112,0,28},  /*112*/
  {112,0,0},    {112,28,0},
  {112,56,0},    {112,84,0},
  {112,112,0},    {84,112,0},
  {56,112,0},    {28,112,0},
  {0,112,0},    {0,112,28},
  {0,112,56},    {0,112,84},
  {0,112,112},    {0,84,112},
  {0,56,112},    {0,28,112},   /*128*/
  {56,56,112},    {68,56,112},
  {84,56,112},    {96,56,112},
  {112,56,112},    {112,56,96},
  {112,56,84},    {112,56,68},
  {112,56,56},    {112,68,56},
  {112,84,56},    {112,96,56},
  {112,112,56},    {96,112,56},
  {84,112,56},    {68,112,56},  /*144*/
  {56,112,56},    {56,112,69},
  {56,112,84},    {56,112,96},
  {56,112,112},    {56,96,112},
  {56,84,112},    {56,68,112},
  {80,80,112},    {88,80,112},
  {96,80,112},    {104,80,112},
  {112,80,112},    {112,80,104},
  {112,80,96},    {112,80,88},  /*160*/
  {112,80,80},    {112,88,80},
  {112,96,80},    {112,104,80},
  {112,112,80},    {104,112,80},
  {96,112,80},    {88,112,80},
  {80,112,80},    {80,112,88},
  {80,112,96},    {80,112,104},
  {80,112,112},    {80,114,112},
  {80,96,112},    {80,88,112},  /*176*/
  {0,0,64},    {16,0,64},
  {32,0,64},    {48,0,64},
  {64,0,64},    {64,0,48},
  {64,0,32},    {64,0,16},
  {64,0,0},    {64,16,0},
  {64,32,0},    {64,48,0},
  {64,64,0},    {48,64,0},
  {32,64,0},    {16,64,0},  /*192*/
  {0,64,0},    {0,64,16},
  {0,64,32},    {0,64,48},
  {0,64,64},    {0,48,64},
  {0,32,64},    {0,16,64},
  {32,32,64},    {40,32,64},
  {48,32,64},    {56,32,64},
  {64,32,64},    {64,32,56},
  {64,32,48},    {64,32,40},  /*208*/
  {64,32,32},    {64,40,32},
  {64,48,32},    {64,56,32},
  {64,64,32},    {56,64,32},
  {48,64,32},    {40,64,32},
  {32,64,32},    {32,64,40},
  {32,64,48},    {32,64,56},
  {32,64,64},    {32,56,64},
  {32,48,64},    {32,40,64},  /*224*/
  {44,44,64},    {48,44,64},
  {52,44,64},    {60,44,64},
  {64,44,64},    {64,44,60},
  {64,44,52},    {64,44,48},
  {64,44,44},    {64,48,44},
  {64,52,44},    {64,60,44},
  {64,64,44},    {60,64,44},
  {52,64,44},    {48,64,44},  /*240*/
  {44,64,44},    {44,64,48},
  {44,64,52},    {44,64,60},
  {44,64,64},    {44,60,64},
  {44,55,64},    {44,48,64},
  {0,0,0},    {0,0,0},
  {0,0,0},    {0,0,0},
  {0,0,0},    {0,0,0},
  {0,0,0},    {0,0,0}    /*256*/
};

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s W P G                                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsWPG() returns True if the image format type, identified by the magick
%  string, is WPG.
%
%  The format of the IsWPG method is:
%
%      unsigned int IsWPG(const unsigned char *magick,const size_t length)
%
%  A description of each parameter follows:
%
%    o status:  Method IsWPG returns True if the image format type is WPG.
%
%    o magick: compare image format pattern against these bytes.
%
%    o length: Specifies the length of the magick string.
%
*/
static unsigned int IsWPG(const unsigned char *magick,const size_t length)
{
  if (length < 4)
    return(MagickFalse);
  if (memcmp(magick,"\377WPC",4) == 0)
    return(MagickTrue);
  return(MagickFalse);
}


static int Rd_WP_DWORD(Image *image,size_t *d)
{
  unsigned char
    b;

  b=ReadBlobByte(image);
  *d=b;
  if (b < 0xFFU)
    return(1);
  b=ReadBlobByte(image);
  *d=(size_t) b;
  b=ReadBlobByte(image);
  *d+=(size_t) b*256l;
  if (*d < 0x8000)
    return(3);
  *d=(*d & 0x7FFF) << 16;
  b=ReadBlobByte(image);
  *d+=(size_t) b;
  b=ReadBlobByte(image);
  *d+=(size_t) b*256l;
  return(5);
}

static MagickBooleanType InsertRow(Image *image,unsigned char *p,ssize_t y,
  int bpp,ExceptionInfo *exception)
{
  int
    bit;

  Quantum
    index,
    *q;

  ssize_t
    x;

  q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
  if (q == (Quantum *) NULL)
    return(MagickFalse);
  switch (bpp)
    {
    case 1:  /* Convert bitmap scanline. */
      {
        for (x=0; x < ((ssize_t) image->columns-7); x+=8)
        {
          for (bit=0; bit < 8; bit++)
          {
            index=((*p) & (0x80 >> bit) ? 0x01 : 0x00);
            SetPixelIndex(image,index,q);
            if (index < image->colors)
              SetPixelViaPixelInfo(image,image->colormap+(ssize_t) index,q);
            q+=GetPixelChannels(image);
          }
          p++;
        }
        if ((image->columns % 8) != 0)
          {
            for (bit=0; bit < (ssize_t) (image->columns % 8); bit++)
            {
              index=((*p) & (0x80 >> bit) ? 0x01 : 0x00);
              SetPixelIndex(image,index,q);
              if (index < image->colors)
                SetPixelViaPixelInfo(image,image->colormap+(ssize_t) index,q);
              q+=GetPixelChannels(image);
            }
            p++;
          }
        break;
      }
    case 2:  /* Convert PseudoColor scanline. */
      {
        for (x=0; x < ((ssize_t) image->columns-3); x+=4)
        {
            index=ConstrainColormapIndex(image,(*p >> 6) & 0x3,exception);
            SetPixelIndex(image,index,q);
            if (index < image->colors)
              SetPixelViaPixelInfo(image,image->colormap+(ssize_t) index,q);
            q+=GetPixelChannels(image);
            index=ConstrainColormapIndex(image,(*p >> 4) & 0x3,exception);
            SetPixelIndex(image,index,q);
            if (index < image->colors)
              SetPixelViaPixelInfo(image,image->colormap+(ssize_t) index,q);
            q+=GetPixelChannels(image);
            index=ConstrainColormapIndex(image,(*p >> 2) & 0x3,exception);
            SetPixelIndex(image,index,q);
            if (index < image->colors)
              SetPixelViaPixelInfo(image,image->colormap+(ssize_t) index,q);
            q+=GetPixelChannels(image);
            index=ConstrainColormapIndex(image,(*p) & 0x3,exception);
            SetPixelIndex(image,index,q);
            if (index < image->colors)
              SetPixelViaPixelInfo(image,image->colormap+(ssize_t) index,q);
            q+=GetPixelChannels(image);
            p++;
        }
       if ((image->columns % 4) != 0)
          {
            index=ConstrainColormapIndex(image,(*p >> 6) & 0x3,exception);
            SetPixelIndex(image,index,q);
            if (index < image->colors)
              SetPixelViaPixelInfo(image,image->colormap+(ssize_t) index,q);
            q+=GetPixelChannels(image);
            if ((image->columns % 4) > 1)
              {
                index=ConstrainColormapIndex(image,(*p >> 4) & 0x3,exception);
                SetPixelIndex(image,index,q);
                if (index < image->colors)
                  SetPixelViaPixelInfo(image,image->colormap+(ssize_t) index,q);
                q+=GetPixelChannels(image);
                if ((image->columns % 4) > 2)
                  {
                    index=ConstrainColormapIndex(image,(*p >> 2) & 0x3,
                      exception);
                    SetPixelIndex(image,index,q);
                    if (index < image->colors)
                      SetPixelViaPixelInfo(image,image->colormap+(ssize_t)
                        index,q);
                    q+=GetPixelChannels(image);
                  }
              }
            p++;
          }
        break;
      }

    case 4:  /* Convert PseudoColor scanline. */
      {
        for (x=0; x < ((ssize_t) image->columns-1); x+=2)
          {
            index=ConstrainColormapIndex(image,(*p >> 4) & 0x0f,exception);
            SetPixelIndex(image,index,q);
            if (index < image->colors)
              SetPixelViaPixelInfo(image,image->colormap+(ssize_t) index,q);
            q+=GetPixelChannels(image);
            index=ConstrainColormapIndex(image,(*p) & 0x0f,exception);
            SetPixelIndex(image,index,q);
            if (index < image->colors)
              SetPixelViaPixelInfo(image,image->colormap+(ssize_t) index,q);
            p++;
            q+=GetPixelChannels(image);
          }
        if ((image->columns % 2) != 0)
          {
            index=ConstrainColormapIndex(image,(*p >> 4) & 0x0f,exception);
            SetPixelIndex(image,index,q);
            if (index < image->colors)
              SetPixelViaPixelInfo(image,image->colormap+(ssize_t) index,q);
            p++;
            q+=GetPixelChannels(image);
          }
        break;
      }
    case 8: /* Convert PseudoColor scanline. */
      {
        for (x=0; x < (ssize_t) image->columns; x++)
          {
            index=ConstrainColormapIndex(image,*p,exception);
            SetPixelIndex(image,index,q);
            if (index < image->colors)
              SetPixelViaPixelInfo(image,image->colormap+(ssize_t) index,q);
            p++;
            q+=GetPixelChannels(image);
          }
      }
      break;

    case 24:     /*  Convert DirectColor scanline.  */
      for (x=0; x < (ssize_t) image->columns; x++)
        {
          SetPixelRed(image,ScaleCharToQuantum(*p++),q);
          SetPixelGreen(image,ScaleCharToQuantum(*p++),q);
          SetPixelBlue(image,ScaleCharToQuantum(*p++),q);
          q+=GetPixelChannels(image);
        }
      break;
    default:
      return(MagickFalse);
    }
  if (!SyncAuthenticPixels(image,exception))
    return(MagickFalse);
  return(MagickTrue);
}


/* Helper for WPG1 raster reader. */
#define InsertByte(b) \
{ \
  BImgBuff[x]=b; \
  x++; \
  if((ssize_t) x>=ldblk) \
  { \
    if (InsertRow(image,BImgBuff,(ssize_t) y,bpp,exception) != MagickFalse) \
      y++; \
    x=0; \
  } \
}
/* WPG1 raster reader. */
static int UnpackWPGRaster(Image *image,int bpp,ExceptionInfo *exception)
{
  int
    x,
    y,
    i;

  unsigned char
    bbuf,
    *BImgBuff,
    RunCount;

  ssize_t
    ldblk;

  x=0;
  y=0;

  ldblk=(ssize_t) ((bpp*(ssize_t) image->columns+7)/8);
  BImgBuff=(unsigned char *) AcquireQuantumMemory((size_t) ldblk,
    8*sizeof(*BImgBuff));
  if(BImgBuff==NULL) return(-2);
  (void) memset(BImgBuff,0,(size_t) ldblk*8*sizeof(*BImgBuff));
  while (y < (ssize_t) image->rows)
  {
      int
        c;

      c=ReadBlobByte(image);
      if (c == EOF)
        break;
      bbuf=(unsigned char) c;
      RunCount=bbuf & 0x7F;
      if(bbuf & 0x80)
        {
          if(RunCount)  /* repeat next byte runcount * */
            {
              bbuf=ReadBlobByte(image);
              for(i=0;i<(int) RunCount;i++) InsertByte(bbuf);
            }
          else {  /* read next byte as RunCount; repeat 0xFF runcount* */
            c=ReadBlobByte(image);
            if (c < 0)
              break;
            RunCount=(unsigned char) c;
            for(i=0;i<(int) RunCount;i++) InsertByte(0xFF);
          }
        }
      else {
        if(RunCount)   /* next runcount byte are read directly */
          {
            for(i=0;i < (int) RunCount;i++)
              {
                c=ReadBlobByte(image);
                if (c < 0)
                  break;
                InsertByte(c);
              }
          }
        else {  /* repeat previous line runcount* */
          c=ReadBlobByte(image);
          if (c == EOF)
            {
              BImgBuff=(unsigned char *) RelinquishMagickMemory(BImgBuff);
              return(-7);
            }
          RunCount=(unsigned char) c;
          if(x!=0) {    /* attempt to duplicate row from x position: */
            if (InsertRow(image,BImgBuff,y,bpp,exception) == MagickFalse)
              {
                x=0;    
                y++;
              }
            BImgBuff=(unsigned char *) RelinquishMagickMemory(BImgBuff);
            return(-3);
          }
          for (i=0; i < (int) RunCount; i++)
          {
            if (y >= (ssize_t) image->rows)
              {
                BImgBuff=(unsigned char *) RelinquishMagickMemory(BImgBuff);
                return(-4);
              }
            if (InsertRow(image,BImgBuff,y,bpp,exception) == MagickFalse)
              {
                BImgBuff=(unsigned char *) RelinquishMagickMemory(BImgBuff);
                return(-6);
              }
            y++;
          }
        }
      }
      if (EOFBlob(image) != MagickFalse)
        break;
    }
  BImgBuff=(unsigned char *) RelinquishMagickMemory(BImgBuff);
  return(y <(ssize_t) image->rows ? -5 : 0);
}


/* Helper for WPG2 reader. */
#define InsertByte6(b) \
{ \
DisableMSCWarning(4310) \
  if(XorMe)\
    BImgBuff[x] = (unsigned char)~b;\
  else\
    BImgBuff[x] = b;\
RestoreMSCWarning \
  x++; \
  if((ssize_t) x >= ldblk) \
  { \
    if (InsertRow(image,BImgBuff,(ssize_t) y,bpp,exception) != MagickFalse) \
      y++; \
    x=0; \
   } \
}
/* WPG2 raster reader. */
static int UnpackWPG2Raster(Image *image,int bpp,ExceptionInfo *exception)
{
  int
    RunCount,
    XorMe = 0;

  ssize_t
    i,
    ldblk,
    x,
    y;

  unsigned int
    SampleSize=1;

  unsigned char
    bbuf,
    *BImgBuff,
    SampleBuffer[] = { 0, 0, 0, 0, 0, 0, 0, 0 };

  x=0;
  y=0;
  ldblk=(ssize_t) ((bpp*(ssize_t) image->columns+7)/8);
  BImgBuff=(unsigned char *) AcquireQuantumMemory((size_t) ldblk,
    8*sizeof(*BImgBuff));
  if(BImgBuff==NULL)
    return(-2);
  (void) memset(BImgBuff,0,((size_t) ldblk*8*sizeof(*BImgBuff)));

  while( y< (ssize_t) image->rows)
  {
      bbuf=ReadBlobByte(image);

      switch(bbuf)
        {
        case 0x7D:
          SampleSize=(unsigned int) ReadBlobByte(image);  /* DSZ */
          if(SampleSize>8)
            {
              BImgBuff=(unsigned char *) RelinquishMagickMemory(BImgBuff);
              return(-2);
            }
          if(SampleSize<1)
            {
              BImgBuff=(unsigned char *) RelinquishMagickMemory(BImgBuff);
              return(-2);
            }
          break;
        case 0x7E:
          if (y == 0)
            (void) FormatLocaleFile(stderr,
              "\nUnsupported WPG token XOR, please report!");
          XorMe=!XorMe;
          break;
        case 0x7F:
          RunCount=ReadBlobByte(image);   /* BLK */
          if (RunCount < 0)
            break;
          for(i=0; i < ((ssize_t) SampleSize*(RunCount+1)); i++)
            {
              InsertByte6(0);
            }
          break;
        case 0xFD:
          RunCount=ReadBlobByte(image);   /* EXT */
          if (RunCount < 0)
            break;
          for(i=0; i<= RunCount;i++)
            for(bbuf=0; bbuf < SampleSize; bbuf++)
              InsertByte6(SampleBuffer[bbuf]);
          break;
        case 0xFE:
          RunCount=ReadBlobByte(image);  /* RST */
          if (RunCount < 0)
            break;
          if(x!=0)
            {
              (void) FormatLocaleFile(stderr,
                "\nUnsupported WPG2 unaligned token RST x=%.20g, please report!\n"
                ,(double) x);
              BImgBuff=(unsigned char *) RelinquishMagickMemory(BImgBuff);
              return(-3);
            }
          {
            /* duplicate the previous row RunCount x */
            for(i=0;i<=RunCount;i++)
              {
                if (InsertRow(image,BImgBuff,((ssize_t) image->rows > y ? y : (ssize_t) image->rows-1),bpp,exception) == MagickFalse)
                  {
                    BImgBuff=(unsigned char *) RelinquishMagickMemory(BImgBuff);
                    return(-3);
                  }
                y++;
              }
          }
          break;
        case 0xFF:
          RunCount=ReadBlobByte(image);   /* WHT */
          if (RunCount < 0)
            break;
          for(i=0; i < ((ssize_t) SampleSize*(RunCount+1)); i++)
            {
              InsertByte6(0xFF);
            }
          break;
        default:
          RunCount=bbuf & 0x7F;

          if(bbuf & 0x80)     /* REP */
            {
              for(i=0; i < SampleSize; i++)
                SampleBuffer[i]=ReadBlobByte(image);
              for(i=0;i<=RunCount;i++)
                for(bbuf=0;bbuf<SampleSize;bbuf++)
                  InsertByte6(SampleBuffer[bbuf]);
            }
          else {      /* NRP */
            for(i=0; i < (ssize_t) ((int) SampleSize*((int) RunCount+1)); i++)
              {
                bbuf=ReadBlobByte(image);
                InsertByte6(bbuf);
              }
          }
        }
      if (EOFBlob(image) != MagickFalse)
        break;
    }
  BImgBuff=(unsigned char *) RelinquishMagickMemory(BImgBuff);
  return(y < (ssize_t) image->rows ? -5 : 0);
}


typedef float tCTM[3][3];

static unsigned LoadWPG2Flags(Image *image,char Precision,float *Angle,tCTM *CTM)
{
const unsigned char TPR=1,TRN=2,SKW=4,SCL=8,ROT=0x10,OID=0x20,LCK=0x80;
ssize_t x;
unsigned DenX;
unsigned Flags;

 (void) memset(*CTM,0,sizeof(*CTM));     /*CTM.erase();CTM.resize(3,3);*/
 (*CTM)[0][0]=1;
 (*CTM)[1][1]=1;
 (*CTM)[2][2]=1;

 Flags=ReadBlobLSBShort(image);
 if(Flags & LCK) (void) ReadBlobLSBLong(image);  /*Edit lock*/
 if(Flags & OID)
  {
    /* Read object ID. */
    if (Precision == 0)
      {
        x=ReadBlobLSBShort(image);
        if (x >= 0x8000)
          {
            Precision=1;
            (void) ReadBlobLSBShort(image);
          }
      }
    else
      (void) ReadBlobLSBLong(image);
  }
 if(Flags & ROT)
  {
  x=ReadBlobLSBLong(image);  /*Rot Angle*/
  if(Angle) *Angle=x/65536.0;
  }
 if(Flags & (ROT|SCL))
  {
  x=ReadBlobLSBLong(image);  /*Sx*cos()*/
  (*CTM)[0][0] = (float)x/0x10000;
  x=ReadBlobLSBLong(image);  /*Sy*cos()*/
  (*CTM)[1][1] = (float)x/0x10000;
  }
 if(Flags & (ROT|SKW))
  {
  x=ReadBlobLSBLong(image);       /*Kx*sin()*/
  (*CTM)[1][0] = (float)x/0x10000;
  x=ReadBlobLSBLong(image);       /*Ky*sin()*/
  (*CTM)[0][1] = (float)x/0x10000;
  }
 if(Flags & TRN)
  {
  x=ReadBlobLSBLong(image); DenX=ReadBlobLSBShort(image);  /*Tx*/
        if(x>=0) (*CTM)[0][2] = (float)x+(float)DenX/0x10000;
            else (*CTM)[0][2] = (float)x-(float)DenX/0x10000;
  x=ReadBlobLSBLong(image); DenX=ReadBlobLSBShort(image);  /*Ty*/
  (*CTM)[1][2]=(float)x + ((x>=0)?1:-1)*(float)DenX/0x10000;
        if(x>=0) (*CTM)[1][2] = (float)x+(float)DenX/0x10000;
            else (*CTM)[1][2] = (float)x-(float)DenX/0x10000;
  }
 if(Flags & TPR)
  {
  x=ReadBlobLSBShort(image); DenX=ReadBlobLSBShort(image);  /*Px*/
  (*CTM)[2][0] = x + (float)DenX/0x10000;;
  x=ReadBlobLSBShort(image);  DenX=ReadBlobLSBShort(image); /*Py*/
  (*CTM)[2][1] = x + (float)DenX/0x10000;
  }
 return(Flags);
}


static Image *ExtractPostscript(Image *image,const ImageInfo *image_info,
  MagickOffsetType PS_Offset,ssize_t PS_Size,ExceptionInfo *exception)
{
  char
    postscript_file[MagickPathExtent];

  const MagicInfo
    *magic_info;

  FILE
    *ps_file;

  int
    c;

  ImageInfo
    *clone_info;

  Image
    *image2;

  MagickBooleanType
    status;

  unsigned char
    magick[2*MagickPathExtent];

  ssize_t
    count;

  if ((clone_info=CloneImageInfo(image_info)) == NULL)
    return(image);
  clone_info->blob=(void *) NULL;
  clone_info->length=0;
  status=MagickFalse;

  /* Obtain temporary file */
  (void) AcquireUniqueFilename(postscript_file);
  ps_file=fopen_utf8(postscript_file,"wb");
  if (ps_file == (FILE *) NULL)
    goto FINISH;

  /* Copy postscript to temporary file */
  if (SeekBlob(image,PS_Offset,SEEK_SET) != PS_Offset)
    {
      (void) fclose(ps_file);
      ThrowException(exception,CorruptImageError,"ImproperImageHeader",
        image->filename);
      goto FINISH_UNL;
    }
  count=ReadBlob(image, 2*MagickPathExtent, magick);
  if (count < 1)
    {
      (void) fclose(ps_file);
      ThrowException(exception,CorruptImageError,"ImproperImageHeader",
        image->filename);
      goto FINISH_UNL;
    }

  if (SeekBlob(image,PS_Offset,SEEK_SET) != PS_Offset)
    {
      (void) fclose(ps_file);
      ThrowException(exception,CorruptImageError,"ImproperImageHeader",
        image->filename);
      goto FINISH_UNL;
    }
  while (PS_Size-- > 0)
  {
    c=ReadBlobByte(image);
    if (c == EOF)
      {
        (void) fclose(ps_file);
        ThrowException(exception,CorruptImageError,"ImproperImageHeader",
          image->filename);
        goto FINISH_UNL;
      }
    (void) fputc(c,ps_file);
  }
  (void) fclose(ps_file);

    /* Detect file format - Check magic.mgk configuration file. */
  magic_info=GetMagicInfo(magick,(size_t) count,exception);
  if(magic_info == (const MagicInfo *) NULL) goto FINISH_UNL;
  if(exception->severity != UndefinedException) goto FINISH_UNL;
  (void) CopyMagickString(clone_info->magick,GetMagicName(magic_info),
    MagickPathExtent);
  if ((LocaleCompare(clone_info->magick,"PFB") != 0) ||
      (LocaleCompare(clone_info->magick,"8BIMTEXT") != 0))
    {
      ThrowException(exception,CorruptImageError,
        "DataStorageTypeIsNotSupported",image->filename);
      goto FINISH_UNL;
    }

  /* Read nested image */
  FormatLocaleString(clone_info->filename,MagickPathExtent,"ps:%.1024s",
    postscript_file);
  image2=ReadImage(clone_info,exception);
  if (!image2)
    goto FINISH_UNL;
  if (exception->severity >= ErrorException)
    {
      CloseBlob(image2);
      DestroyImageList(image2);
      goto FINISH_UNL;
    }

  {
    Image
      *p;

    /*
      Replace current image with new image while copying base image attributes.
    */
    p=image2;
    do
    {
      (void) CopyMagickString(p->filename,image->filename,MagickPathExtent);
      (void) CopyMagickString(p->magick_filename,image->magick_filename,
        MagickPathExtent);
      (void) CopyMagickString(p->magick,image->magick,MagickPathExtent);
      if ((p->rows == 0) || (p->columns == 0))
        {
          DeleteImageFromList(&p);
          if (p == (Image *) NULL)
            {
              image2=(Image *) NULL;
              goto FINISH_UNL;
            }
        }
      else
        {
          DestroyBlob(p);
          p->blob=ReferenceBlob(image->blob);
          p=p->next;
        }
    } while (p != (Image *) NULL);
  }

  if ((image->rows == 0 || image->columns == 0) &&
      (image->previous != NULL || image->next != NULL))
  {
    DeleteImageFromList(&image);
  }

  AppendImageToList(&image,image2);
  while (image->next != NULL)
    image=image->next;
  status=MagickTrue;

 FINISH_UNL:
  (void) RelinquishUniqueFileResource(postscript_file);
 FINISH:
  DestroyImageInfo(clone_info);
  if (status == MagickFalse)
    return(DestroyImageList(image));
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d W P G I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method ReadWPGImage reads an WPG X image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadWPGImage method is:
%
%    Image *ReadWPGImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image:  Method ReadWPGImage returns a pointer to the image after
%      reading. A null image is returned if there is a memory shortage or if
%      the image cannot be read.
%
%    o image_info: Specifies a pointer to a ImageInfo structure.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadWPGImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
  typedef struct
  {
    size_t FileId;
    MagickOffsetType DataOffset;
    unsigned char ProductType;
    unsigned char FileType;
    unsigned char MajorVersion;
    unsigned char MinorVersion;
    unsigned int EncryptKey;
    unsigned int Reserved;
  } WPGHeader;

  typedef struct
  {
    unsigned char RecType;
    size_t RecordLength;
  } WPGRecord;

  typedef struct
  {
    unsigned char Class;
    unsigned char RecType;
    size_t Extension;
    size_t RecordLength;
  } WPG2Record;

  typedef struct
  {
    unsigned  HorizontalUnits;
    unsigned  VerticalUnits;
    unsigned char PosSizePrecision;
  } WPG2Start;

  typedef struct
  {
    unsigned int Width;
    unsigned int Height;
    unsigned int Depth;
    unsigned int HorzRes;
    unsigned int VertRes;
  } WPGBitmapType1;

  typedef struct
  {
    unsigned int Width;
    unsigned int Height;
    unsigned char Depth;
    unsigned char Compression;
  } WPG2BitmapType1;

  typedef struct
  {
    unsigned int RotAngle;
    unsigned int LowLeftX;
    unsigned int LowLeftY;
    unsigned int UpRightX;
    unsigned int UpRightY;
    unsigned int Width;
    unsigned int Height;
    unsigned int Depth;
    unsigned int HorzRes;
    unsigned int VertRes;
  } WPGBitmapType2;

  typedef struct
  {
    unsigned int StartIndex;
    unsigned int NumOfEntries;
  } WPGColorMapRec;

  /*
  typedef struct {
    size_t PS_unknown1;
    unsigned int PS_unknown2;
    unsigned int PS_unknown3;
  } WPGPSl1Record;
  */

  Image
    *image;

  unsigned int
    status;

  WPGHeader
    Header;

  WPGRecord
    Rec;

  WPG2Record
    Rec2;

  WPG2Start StartWPG;

  WPGBitmapType1
    BitmapHeader1;

  WPG2BitmapType1
    Bitmap2Header1;

  WPGBitmapType2
    BitmapHeader2;

  WPGColorMapRec
    WPG_Palette;

  int
    i,
    bpp,
    WPG2Flags;

  ssize_t
    ldblk;

  size_t
    one;

  unsigned char
    *BImgBuff;

  tCTM CTM;         /*current transform matrix*/

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  one=1;
  image=AcquireImage(image_info,exception);
  image->depth=8;
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  /*
    Read WPG image.
  */
  Header.FileId=ReadBlobLSBLong(image);
  Header.DataOffset=(MagickOffsetType) ReadBlobLSBLong(image);
  Header.ProductType=ReadBlobByte(image);
  Header.FileType=ReadBlobByte(image);
  Header.MajorVersion=ReadBlobByte(image);
  Header.MinorVersion=ReadBlobByte(image);
  Header.EncryptKey=ReadBlobLSBShort(image);
  Header.Reserved=ReadBlobLSBShort(image);

  if ((Header.FileId != 0x435057FF) || (Header.FileType != 0x16))
    ThrowReaderException(CorruptImageError,"ImproperImageHeader");
  if (Header.EncryptKey!=0)
    ThrowReaderException(CoderError,"EncryptedWPGImageFileNotSupported");

  image->colors=0;
  image->storage_class=DirectClass;
  bpp=0;
  BitmapHeader2.RotAngle=0;
  Rec2.RecordLength=0;
  switch(Header.MajorVersion)
  {
    case 1:     /* WPG level 1 */
      while(!EOFBlob(image)) /* object parser loop */
        {
          if (SeekBlob(image,Header.DataOffset,SEEK_SET) != Header.DataOffset)
            break;
          if (EOFBlob(image))
            break;
          Rec.RecType=(i=ReadBlobByte(image));
          if (i==EOF)
            break;
          i=Rd_WP_DWORD(image,&Rec.RecordLength);
          if ((Rec.RecordLength+4) >= GetBlobSize(image))
            ThrowReaderException(CorruptImageError,"ImproperImageHeader");
          if (EOFBlob(image))
            break;
          Header.DataOffset=(MagickOffsetType) TellBlob(image)+(ssize_t)
            Rec.RecordLength;
          if (Header.DataOffset > (MagickOffsetType) GetBlobSize(image))
            ThrowReaderException(CorruptImageError, 
              "InsufficientImageDataInFile");
          switch(Rec.RecType)
            {
            case 0x0B: /* bitmap type 1 */
              BitmapHeader1.Width=ReadBlobLSBShort(image);
              BitmapHeader1.Height=ReadBlobLSBShort(image);
              if ((BitmapHeader1.Width == 0) || (BitmapHeader1.Height == 0))
                ThrowReaderException(CorruptImageError,"ImproperImageHeader");
              BitmapHeader1.Depth=ReadBlobLSBShort(image);
              BitmapHeader1.HorzRes=ReadBlobLSBShort(image);
              BitmapHeader1.VertRes=ReadBlobLSBShort(image);

              if(BitmapHeader1.HorzRes && BitmapHeader1.VertRes)
                {
                  image->units=PixelsPerCentimeterResolution;
                  image->resolution.x=BitmapHeader1.HorzRes/470.0;
                  image->resolution.y=BitmapHeader1.VertRes/470.0;
                }
              image->columns=BitmapHeader1.Width;
              image->rows=BitmapHeader1.Height;
              bpp=(int) BitmapHeader1.Depth;
              if ((bpp == 1) &&
                  (AcquireImageColormap(image,2,exception) == MagickFalse))
                goto NoMemory;
              goto UnpackRaster;

            case 0x0E:  /*Color palette */
              WPG_Palette.StartIndex=ReadBlobLSBShort(image);
              WPG_Palette.NumOfEntries=ReadBlobLSBShort(image);
              if ((WPG_Palette.NumOfEntries-WPG_Palette.StartIndex) >
                  (Rec2.RecordLength-2-2)/3)
                ThrowReaderException(CorruptImageError,"InvalidColormapIndex");
              if (WPG_Palette.StartIndex > WPG_Palette.NumOfEntries)
                ThrowReaderException(CorruptImageError,"InvalidColormapIndex");
              image->colors=WPG_Palette.NumOfEntries;
              if (AcquireImageColormap(image,image->colors,exception) == MagickFalse)
                goto NoMemory;
              for (i=(int) WPG_Palette.StartIndex;
                   i < (int) WPG_Palette.NumOfEntries; i++)
                {
                  image->colormap[i].red=ScaleCharToQuantum((unsigned char)
                    ReadBlobByte(image));
                  image->colormap[i].green=ScaleCharToQuantum((unsigned char)
                    ReadBlobByte(image));
                  image->colormap[i].blue=ScaleCharToQuantum((unsigned char)
                    ReadBlobByte(image));
                }
              break;

            case 0x11:  /* Start PS l1 */
              if (Rec.RecordLength > 8)
                {
                  image=ExtractPostscript(image,image_info,
                    TellBlob(image)+8,   /* skip PS header in the wpg */
                    (ssize_t) Rec.RecordLength-8,exception);
                  if (image == NULL)
                    ThrowReaderException(CorruptImageError,
                      "ImproperImageHeader");
                }
              break;

            case 0x14:  /* bitmap type 2 */
              BitmapHeader2.RotAngle=ReadBlobLSBShort(image);
              BitmapHeader2.LowLeftX=ReadBlobLSBShort(image);
              BitmapHeader2.LowLeftY=ReadBlobLSBShort(image);
              BitmapHeader2.UpRightX=ReadBlobLSBShort(image);
              BitmapHeader2.UpRightY=ReadBlobLSBShort(image);
              BitmapHeader2.Width=ReadBlobLSBShort(image);
              BitmapHeader2.Height=ReadBlobLSBShort(image);
              if ((BitmapHeader2.Width == 0) || (BitmapHeader2.Height == 0))
                ThrowReaderException(CorruptImageError,"ImproperImageHeader");
              BitmapHeader2.Depth=ReadBlobLSBShort(image);
              BitmapHeader2.HorzRes=ReadBlobLSBShort(image);
              BitmapHeader2.VertRes=ReadBlobLSBShort(image);

              image->units=PixelsPerCentimeterResolution;
              image->page.width=(unsigned int)
                ((BitmapHeader2.LowLeftX-BitmapHeader2.UpRightX)/470.0);
              image->page.height=(unsigned int)
                ((BitmapHeader2.LowLeftX-BitmapHeader2.UpRightY)/470.0);
              image->page.x=(int) (BitmapHeader2.LowLeftX/470.0);
              image->page.y=(int) (BitmapHeader2.LowLeftX/470.0);
              if(BitmapHeader2.HorzRes && BitmapHeader2.VertRes)
                {
                  image->resolution.x=BitmapHeader2.HorzRes/470.0;
                  image->resolution.y=BitmapHeader2.VertRes/470.0;
                }
              image->columns=BitmapHeader2.Width;
              image->rows=BitmapHeader2.Height;
              bpp=(int) BitmapHeader2.Depth;

            UnpackRaster:
              status=SetImageExtent(image,image->columns,image->rows,exception);
              if (status == MagickFalse)
                break;
              (void) ResetImagePixels(image,exception);
              if ((image->storage_class != PseudoClass) && (bpp < 24))
                {
                  image->colors=one << bpp;
                  if (image->colors > GetBlobSize(image))
                    ThrowReaderException(CorruptImageError,
                      "InsufficientImageDataInFile");
                  if (!AcquireImageColormap(image,image->colors,exception))
                    {
                    NoMemory:
                      ThrowReaderException(ResourceLimitError,
                        "MemoryAllocationFailed");
                    }
                  for (i=0; (i < (int) image->colors) && (i < 256); i++)
                    {
                      image->colormap[i].red=ScaleCharToQuantum(WPG1_Palette[i].Red);
                      image->colormap[i].green=ScaleCharToQuantum(WPG1_Palette[i].Green);
                      image->colormap[i].blue=ScaleCharToQuantum(WPG1_Palette[i].Blue);
                      image->colormap[i].alpha=OpaqueAlpha;
                    }
                }
              else
                {
                  if (bpp < 24)
                  if ( (image->colors < (one << bpp)) && (bpp != 24) )
                    {
                      PixelInfo
                        *colormap;

                      size_t
                        colors;

                      colormap=image->colormap;
                      colors=image->colors;
                      image->colormap=(PixelInfo *) NULL;
                      if (AcquireImageColormap(image,one << bpp,exception) == MagickFalse)
                        {
                          colormap=(PixelInfo *)
                            RelinquishMagickMemory(colormap);
                          goto NoMemory;
                        }
                      (void) memcpy(image->colormap,colormap,MagickMin(
                        image->colors,colors)*sizeof(*image->colormap));
                      colormap=(PixelInfo *)
                        RelinquishMagickMemory(colormap);
                    }
                }

              if (bpp == 1)
                {
                  image->colormap[0].red=image->colormap[0].green=
                    image->colormap[0].blue=0;
                  image->colormap[0].alpha=OpaqueAlpha;
                  image->colormap[1].red=image->colormap[1].green=
                    image->colormap[1].blue=QuantumRange;
                  image->colormap[1].alpha=OpaqueAlpha;
                }
              if(!image_info->ping)
                if(UnpackWPGRaster(image,bpp,exception) < 0)
                  /* The raster cannot be unpacked */
                  {
                  DecompressionFailed:
                    ThrowReaderException(CoderError,"UnableToDecompressImage");
                  }

              if(Rec.RecType==0x14 && BitmapHeader2.RotAngle!=0 && !image_info->ping)
                {
                  /* flop command */
                  if(BitmapHeader2.RotAngle & 0x8000)
                    {
                      Image
                        *flop_image;

                      flop_image = FlopImage(image, exception);
                      if (flop_image != (Image *) NULL) {
                        DuplicateBlob(flop_image,image);
                        ReplaceImageInList(&image,flop_image);
                      }
                    }
                  /* flip command */
                  if(BitmapHeader2.RotAngle & 0x2000)
                    {
                      Image
                        *flip_image;

                      flip_image = FlipImage(image, exception);
                      if (flip_image != (Image *) NULL) {
                        DuplicateBlob(flip_image,image);
                        ReplaceImageInList(&image,flip_image);
                      }
                    }
                  /* rotate command */
                  if(BitmapHeader2.RotAngle & 0x0FFF)
                    {
                      Image
                        *rotate_image;

                      rotate_image=RotateImage(image,(BitmapHeader2.RotAngle &
                        0x0FFF), exception);
                      if (rotate_image != (Image *) NULL) {
                        DuplicateBlob(rotate_image,image);
                        ReplaceImageInList(&image,rotate_image);
                      }
                    }
                }

              /* Allocate next image structure. */
              if ((image_info->ping != MagickFalse) &&
                  (image_info->number_scenes != 0))
                if (image->scene >= (image_info->scene+image_info->number_scenes-1))
                  goto Finish;
              AcquireNextImage(image_info,image,exception);
              image->depth=8;
              if (image->next == (Image *) NULL)
                goto Finish;
              image=SyncNextImageInList(image);
              image->columns=image->rows=0;
              image->colors=0;
              break;

            case 0x1B:  /* Postscript l2 */
              if (Rec.RecordLength>0x3C)
                {
                  image=ExtractPostscript(image,image_info,
                    TellBlob(image)+0x3C,   /* skip PS l2 header in the wpg */
                    (ssize_t) Rec.RecordLength-0x3C,exception);
                  if (image == NULL)
                    ThrowReaderException(CorruptImageError,
                      "ImproperImageHeader");
                }
              break;
            }
        }
      break;

    case 2:  /* WPG level 2 */
      (void) memset(CTM,0,sizeof(CTM));
      StartWPG.PosSizePrecision = 0;
      while(!EOFBlob(image)) /* object parser loop */
        {
          if (SeekBlob(image,Header.DataOffset,SEEK_SET) != Header.DataOffset)
            break;
          if (EOFBlob(image))
            break;

          Rec2.Class=(i=ReadBlobByte(image));
          if(i==EOF)
            break;
          Rec2.RecType=(i=ReadBlobByte(image));
          if(i==EOF)
            break;
          Rd_WP_DWORD(image,&Rec2.Extension);
          Rd_WP_DWORD(image,&Rec2.RecordLength);
          if(EOFBlob(image))
            break;

          Header.DataOffset=(MagickOffsetType) (TellBlob(image)+(ssize_t)
            Rec2.RecordLength);

          switch(Rec2.RecType)
            {
      case 1:
              StartWPG.HorizontalUnits=ReadBlobLSBShort(image);
              StartWPG.VerticalUnits=ReadBlobLSBShort(image);
              StartWPG.PosSizePrecision=ReadBlobByte(image);
              break;
            case 0x0C:    /* Color palette */
              WPG_Palette.StartIndex=ReadBlobLSBShort(image);
              WPG_Palette.NumOfEntries=ReadBlobLSBShort(image);
              if ((WPG_Palette.NumOfEntries-WPG_Palette.StartIndex) >
                  (Rec2.RecordLength-2-2) / 3)
                ThrowReaderException(CorruptImageError,"InvalidColormapIndex");
              if (WPG_Palette.StartIndex >= WPG_Palette.NumOfEntries)
                ThrowReaderException(CorruptImageError,"InvalidColormapIndex");
              image->colors=WPG_Palette.NumOfEntries;
              if (AcquireImageColormap(image,image->colors,exception) == MagickFalse)
                ThrowReaderException(ResourceLimitError,
                  "MemoryAllocationFailed");
              for (i=(int) WPG_Palette.StartIndex;
                   i < (int) WPG_Palette.NumOfEntries; i++)
                {
                  image->colormap[i].red=ScaleCharToQuantum((unsigned char)
                    ReadBlobByte(image));
                  image->colormap[i].green=ScaleCharToQuantum((unsigned char)
                    ReadBlobByte(image));
                  image->colormap[i].blue=ScaleCharToQuantum((unsigned char)
                    ReadBlobByte(image));
                  image->colormap[i].alpha=OpaqueAlpha;
                  (void) ReadBlobByte(image);   /*Opacity??*/
                }
              break;
            case 0x0E:
              Bitmap2Header1.Width=ReadBlobLSBShort(image);
              Bitmap2Header1.Height=ReadBlobLSBShort(image);
              if ((Bitmap2Header1.Width == 0) || (Bitmap2Header1.Height == 0))
                ThrowReaderException(CorruptImageError,"ImproperImageHeader");
              Bitmap2Header1.Depth=ReadBlobByte(image);
              Bitmap2Header1.Compression=ReadBlobByte(image);

              if(Bitmap2Header1.Compression > 1)
                continue; /*Unknown compression method */
              switch(Bitmap2Header1.Depth)
                {
                case 1:
                  bpp=1;
                  break;
                case 2:
                  bpp=2;
                  break;
                case 3:
                  bpp=4;
                  break;
                case 4:
                  bpp=8;
                  break;
                case 8:
                  bpp=24;
                  break;
                default:
                  continue;  /*Ignore raster with unknown depth*/
                }
              image->columns=Bitmap2Header1.Width;
              image->rows=Bitmap2Header1.Height;
              if (image_info->ping != MagickFalse)
                return(image);
              status=SetImageExtent(image,image->columns,image->rows,exception);
              if (status != MagickFalse)
                status=ResetImagePixels(image,exception);
              if (status == MagickFalse)
                break;
              if ((image->colors == 0) && (bpp != 24))
                {
                  image->colors=one << bpp;
                  if (!AcquireImageColormap(image,image->colors,exception))
                    goto NoMemory;
                }
              else
                {
                  if(bpp < 24)
                    if( image->colors<(one << bpp) && bpp!=24 )
                      image->colormap=(PixelInfo *) ResizeQuantumMemory(
                       image->colormap,(size_t) (one << bpp),
                       sizeof(*image->colormap));
                }


              switch(Bitmap2Header1.Compression)
                {
                case 0:    /*Uncompressed raster*/
                  {
                    ldblk=(ssize_t) ((bpp*(ssize_t) image->columns+7)/8);
                    BImgBuff=(unsigned char *) AcquireQuantumMemory((size_t)
                      ldblk+1,sizeof(*BImgBuff));
                    if (BImgBuff == (unsigned char *) NULL)
                      goto NoMemory;
                    for (i=0; i< (ssize_t) image->rows; i++)
                    {
                      ssize_t
                        count;

                      count=ReadBlob(image,(size_t) ldblk,BImgBuff);
                      if (count != ldblk)
                        break;
                      if (InsertRow(image,BImgBuff,i,bpp,exception) == MagickFalse)
                        break;
                    }
                    BImgBuff=(unsigned char *) RelinquishMagickMemory(BImgBuff);
                    if (i < (ssize_t) image->rows)
                      goto DecompressionFailed;
                    break;
                  }
                case 1:    /*RLE for WPG2 */
                  {
                    if( UnpackWPG2Raster(image,bpp,exception) < 0)
                      goto DecompressionFailed;
                    break;
                  }
                }

              if(CTM[0][0]<0 && !image_info->ping)
                {    /*?? RotAngle=360-RotAngle;*/
                  Image
                    *flop_image;

                  flop_image = FlopImage(image, exception);
                  if (flop_image != (Image *) NULL) {
                    DuplicateBlob(flop_image,image);
                    ReplaceImageInList(&image,flop_image);
                  }
                  /* Try to change CTM according to Flip - I am not sure, must be checked.
                     Tx(0,0)=-1;      Tx(1,0)=0;   Tx(2,0)=0;
                     Tx(0,1)= 0;      Tx(1,1)=1;   Tx(2,1)=0;
                     Tx(0,2)=(WPG._2Rect.X_ur+WPG._2Rect.X_ll);
                     Tx(1,2)=0;   Tx(2,2)=1; */
                }
              if(CTM[1][1]<0 && !image_info->ping)
                {    /*?? RotAngle=360-RotAngle;*/
                  Image
                    *flip_image;

                   flip_image = FlipImage(image, exception);
                   if (flip_image != (Image *) NULL) {
                     DuplicateBlob(flip_image,image);
                     ReplaceImageInList(&image,flip_image);
                    }
                  /* Try to change CTM according to Flip - I am not sure, must be checked.
                     float_matrix Tx(3,3);
                     Tx(0,0)= 1;   Tx(1,0)= 0;   Tx(2,0)=0;
                     Tx(0,1)= 0;   Tx(1,1)=-1;   Tx(2,1)=0;
                     Tx(0,2)= 0;   Tx(1,2)=(WPG._2Rect.Y_ur+WPG._2Rect.Y_ll);
                     Tx(2,2)=1; */
              }


              /* Allocate next image structure. */
              if ((image_info->ping != MagickFalse) &&
                  (image_info->number_scenes != 0))
                if (image->scene >= (image_info->scene+image_info->number_scenes-1))
                  goto Finish;
              AcquireNextImage(image_info,image,exception);
              image->depth=8;
              if (image->next == (Image *) NULL)
                goto Finish;
              image=SyncNextImageInList(image);
              image->columns=image->rows=0;
              image->colors=0;
              break;

            case 0x12:  /* Postscript WPG2*/
        i=ReadBlobLSBShort(image);
              if (Rec2.RecordLength > (unsigned int) i)
                {
                  image=ExtractPostscript(image,image_info,
                    TellBlob(image)+i,    /*skip PS header in the wpg2*/
                    (ssize_t) Rec2.RecordLength-i-2,exception);
                  if (image == NULL)
                    ThrowReaderException(CorruptImageError,
                      "ImproperImageHeader");
                }
              break;

      case 0x1B:          /*bitmap rectangle*/
              WPG2Flags = (int) LoadWPG2Flags(image,(char)
                StartWPG.PosSizePrecision,NULL,&CTM);
              (void) WPG2Flags;
              break;
            }
        }

      break;

    default:
      {
         ThrowReaderException(CoderError,"DataEncodingSchemeIsNotSupported");
      }
   }

 Finish:
  (void) CloseBlob(image);

  {
    Image
      *p;

    ssize_t
      scene=0;

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
      Fix scene numbers.
    */
    for (p=image; p != (Image *) NULL; p=p->next)
      p->scene=(size_t) scene++;
  }
  if (image == (Image *) NULL)
    ThrowReaderException(CorruptImageError,
      "ImageFileDoesNotContainAnyImageData");
  return(image);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r W P G I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method RegisterWPGImage adds attributes for the WPG image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterWPGImage method is:
%
%      size_t RegisterWPGImage(void)
%
*/
ModuleExport size_t RegisterWPGImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("WPG","WPG","Word Perfect Graphics");
  entry->decoder=(DecodeImageHandler *) ReadWPGImage;
  entry->encoder=(EncodeImageHandler *) WriteWPGImage;
  entry->magick=(IsImageFormatHandler *) IsWPG;
  entry->flags^=CoderAdjoinFlag;
  entry->flags|=CoderDecoderSeekableStreamFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r W P G I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  Method UnregisterWPGImage removes format registrations made by the
%  WPG module from the list of supported formats.
%
%  The format of the UnregisterWPGImage method is:
%
%      UnregisterWPGImage(void)
%
*/
ModuleExport void UnregisterWPGImage(void)
{
  (void) UnregisterMagickInfo("WPG");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e W P G I m a g e                                                 %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteWPGImage() writes an image in the WPG format to a file.
%
%  The format of the WriteWPGImage method is:
%
%      MagickBooleanType WriteWPGImage(const ImageInfo *image_info,
%        Image *image,ExceptionInfo *exception)
%
%  A description of each parameter follows.
%
%    o image_info: the image info.
%
%    o image:  The image.
%
%    o exception: return any errors or warnings in this structure.
%
*/

typedef struct
{
	size_t
    count;

	ssize_t
    offset;

	unsigned char
    pixels[256];
} WPGRLEInfo;

static void WPGFlushRLE(WPGRLEInfo *rle_info,Image *image,unsigned char n)
{
  if (n > rle_info->offset)
    n=rle_info->offset;
  if (n > 0x7F)
    n=0x7F;
  if (n > 0)
    {
      (void) WriteBlobByte(image,n);
      (void) WriteBlob(image,n,rle_info->pixels);
      rle_info->offset-=n;
      if (rle_info->offset > 0)
        (void) memmove(rle_info->pixels,rle_info->pixels+n,n);
      else
        rle_info->count=0;
    }
}

static void WPGAddRLEByte(WPGRLEInfo *rle_info,Image *image,
  const unsigned char byte)
{
  rle_info->pixels[rle_info->offset++]=byte;
  if (rle_info->offset > 1)
    {
      if ((rle_info->count == 0x7E) ||
          (rle_info->pixels[rle_info->offset-2] != byte))
        {
          if (rle_info->count >= 1)
            {
              rle_info->count++;
              WPGFlushRLE(rle_info,image,(unsigned char) (rle_info->offset-
                (ssize_t) rle_info->count-1));
              (void) WriteBlobByte(image,(unsigned char) (
                rle_info->count | 0x80));
              (void) WriteBlobByte(image,rle_info->pixels[0]);
              rle_info->offset=1;
              rle_info->pixels[0]=byte;
            }
          rle_info->count = 0;
        }
      else
        rle_info->count++;
  }
  if ((rle_info->offset-(ssize_t) rle_info->count) > 0x7E)
    {
      WPGFlushRLE(rle_info,image,0x7F);
      return;
    }
  if ((rle_info->offset > 0x7E) && (rle_info->count >= 1))
     {
       WPGFlushRLE(rle_info,image,(unsigned char) (rle_info->offset-
         (ssize_t) rle_info->count-1));
       return;
     }
}

static void WPGFlush(WPGRLEInfo *rle_info,Image *image)
{
  if (rle_info->count > 1)
    {
      WPGAddRLEByte(rle_info,image,rle_info->pixels[rle_info->offset-1] ^ 0xFF);
      rle_info->offset=0;
    }
  else
    {
      WPGFlushRLE(rle_info,image,0x7F);
      WPGFlushRLE(rle_info,image,0x7F);
      rle_info->count=0;
    }
}

static void WPGAddRLEBlock(WPGRLEInfo *rle_info,Image *image,
  const unsigned char *pixels,unsigned short extent)
{
  while (extent-- > 0)
  {
    WPGAddRLEByte(rle_info,image,*pixels);
    pixels++;
  }
}

static void WPGInitializeRLE(WPGRLEInfo *rle_info)
{
  rle_info->count=0;
  rle_info->offset=0;
  (void) memset(rle_info->pixels,0,sizeof(rle_info->pixels));
}

static MagickBooleanType WriteWPGImage(const ImageInfo *image_info,Image *image,
  ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  MagickOffsetType
    current_offset,
    offset;

  QuantumInfo
    *quantum_info;

  size_t
    extent;

  ssize_t
    y;

  unsigned char
    *pixels;

  WPGRLEInfo
    rle_info;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
  if (status == MagickFalse)
    return(status);
  if ((image->columns > 65535UL) || (image->rows > 65535UL))
    ThrowWriterException(ImageError,"WidthOrHeightExceedsLimit");
  if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
    (void) TransformImageColorspace(image,sRGBColorspace,exception);
  (void) SetImageType(image,PaletteType,exception);
  /*
    Write WPG header.
  */
  (void) WriteBlobLSBLong(image,0x435057FF);  /* FileId */
  (void) WriteBlobLSBLong(image,16);  /* data offset */
  (void) WriteBlobByte(image,1);  /* product type */
  (void) WriteBlobByte(image,0x16);  /* file type */
  (void) WriteBlobByte(image,1);  /* major version */
  (void) WriteBlobByte(image,0);  /* minor version */
  (void) WriteBlobLSBShort(image,0);  /* encypt key */
  (void) WriteBlobLSBShort(image,0);  /* reserved */
  /*
    Write WPG level 1 header.
  */
  (void) WriteBlobByte(image,0x0f);
  (void) WriteBlobByte(image,0x06);
  (void) WriteBlobByte(image,1);  /* version number */
  (void) WriteBlobByte(image,0);  /* flags */
  (void) WriteBlobLSBShort(image,(unsigned short) image->columns);
  (void) WriteBlobLSBShort(image,(unsigned short) image->rows);
  image->depth=8;
  if (image->colors <= 16)
    image->depth=4;
  if (image->colors <= 2)
    image->depth=1;
  if (image->depth > 1)
    {
      /*
        Write colormap.
      */
      ssize_t i = 0;
      unsigned short number_entries = 0;
      (void) WriteBlobByte(image,0x0e);
      number_entries=3*(1U << image->depth)+4;
      if (number_entries < 0xff)
        (void) WriteBlobByte(image,(unsigned char) number_entries);
      else
        {
          (void) WriteBlobByte(image,0xff);
          (void) WriteBlobLSBShort(image,number_entries);
        }
      (void) WriteBlobLSBShort(image,0); /* start index */
      (void) WriteBlobLSBShort(image,1U << image->depth);
      for ( ; i < (ssize_t) ((size_t) 1U << image->depth); i++)
        if (i >= (ssize_t) image->colors)
          {
            (void) WriteBlobByte(image,i);
            (void) WriteBlobByte(image,i);
            (void) WriteBlobByte(image,i);
          }
        else
          {
            (void) WriteBlobByte(image,ScaleQuantumToChar(
              image->colormap[i].red));
            (void) WriteBlobByte(image,ScaleQuantumToChar(
              image->colormap[i].green));
            (void) WriteBlobByte(image,ScaleQuantumToChar(
              image->colormap[i].blue));
          }
    }
  /*
    Bitmap 1 header.
  */
  (void) WriteBlobByte(image,0x0b);
  (void) WriteBlobByte(image,0xff);
  offset=TellBlob(image);
  (void) WriteBlobLSBShort(image,0x8000);
  (void) WriteBlobLSBShort(image,0);
  (void) WriteBlobLSBShort(image,(unsigned short) image->columns);
  (void) WriteBlobLSBShort(image,(unsigned short) image->rows);
  (void) WriteBlobLSBShort(image,(unsigned char) image->depth);
  (void) WriteBlobLSBShort(image,75);  /* resolution */
  (void) WriteBlobLSBShort(image,75);
  /*
    Write WPG image pixels.
  */
  quantum_info=AcquireQuantumInfo(image_info,image);
  if (quantum_info == (QuantumInfo *) NULL)
    ThrowWriterException(ImageError,"MemoryAllocationFailed");
  pixels=(unsigned char *) GetQuantumPixels(quantum_info);
  extent=GetQuantumExtent(image,quantum_info,image->depth == 1 ? GrayQuantum :
    IndexQuantum);
  (void) memset(pixels,0,extent*sizeof(*pixels));
  WPGInitializeRLE(&rle_info);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    const Quantum
      *p;

    size_t
      length;

    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const Quantum *) NULL)
      break;
    length=ExportQuantumPixels(image,(CacheView *) NULL,quantum_info,
      image->depth == 1 ? GrayQuantum : IndexQuantum,pixels,exception);
    if (length == 0)
      break;
    WPGAddRLEBlock(&rle_info,image,pixels,(unsigned short) length);
    WPGFlush(&rle_info,image);
    status=SetImageProgress(image,SaveImageTag,(MagickOffsetType) y,
      image->rows);
    if (status == MagickFalse)
      break;
  }
  quantum_info=DestroyQuantumInfo(quantum_info);
  current_offset=TellBlob(image);
  (void) WriteBlobByte(image,0x10);
  (void) WriteBlobByte(image,0);
  (void) SeekBlob(image,offset,SEEK_SET);
  offset=current_offset-offset-4;
  (void) WriteBlobLSBShort(image,0x8000 | (offset >> 16));
  (void) WriteBlobLSBShort(image,offset & 0xffff);
  if (y < (ssize_t) image->rows)
    ThrowWriterException(CorruptImageError,"UnableToWriteImageData");
  (void) CloseBlob(image);
  return(status);
}
