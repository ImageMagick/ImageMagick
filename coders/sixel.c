/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                     SSSSS  IIIII  X   X  EEEEE  L                           %
%                     SS       I     X X   E      L                           %
%                      SSS     I      X    EEE    L                           %
%                        SS    I     X X   E      L                           %
%                     SSSSS  IIIII  X   X  EEEEE  LLLLL                       %
%                                                                             %
%                                                                             %
%                        Read/Write DEC SIXEL Format                          %
%                                                                             %
%                              Software Design                                %
%                               Hayaki Saito                                  %
%                              September 2014                                 %
%                    Based on kmiya's sixel (2014-03-28)                      %
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
#include "MagickCore/cache.h"
#include "MagickCore/color.h"
#include "MagickCore/color-private.h"
#include "MagickCore/colormap.h"
#include "MagickCore/colormap-private.h"
#include "MagickCore/colorspace.h"
#include "MagickCore/colorspace-private.h"
#include "MagickCore/exception.h"
#include "MagickCore/exception-private.h"
#include "MagickCore/geometry.h"
#include "MagickCore/image.h"
#include "MagickCore/image-private.h"
#include "MagickCore/list.h"
#include "MagickCore/locale_.h"
#include "MagickCore/magick.h"
#include "MagickCore/memory_.h"
#include "MagickCore/monitor.h"
#include "MagickCore/monitor-private.h"
#include "MagickCore/pixel-accessor.h"
#include "MagickCore/quantize.h"
#include "MagickCore/quantum-private.h"
#include "MagickCore/resize.h"
#include "MagickCore/resource_.h"
#include "MagickCore/splay-tree.h"
#include "MagickCore/static.h"
#include "MagickCore/string_.h"
#include "MagickCore/thread-private.h"
#include "MagickCore/module.h"
#include "MagickCore/threshold.h"
#include "MagickCore/utility.h"

/*
  Definitions
*/
#define SIXEL_PALETTE_MAX 1024
#define SIXEL_OUTPUT_PACKET_SIZE 1024

/*
  Macros
*/
#define SIXEL_RGB(r, g, b) ((int) (((ssize_t) ((r) & 0xff) << 16) + (((g) & 0xff) << 8) +  ((b) & 0xff)))
#define SIXEL_PALVAL(n,a,m) ((int) (((ssize_t) (n) * (a) + ((m) / 2)) / (m)))
#define SIXEL_XRGB(r,g,b) SIXEL_RGB(SIXEL_PALVAL(r, 255, 100), SIXEL_PALVAL(g, 255, 100), SIXEL_PALVAL(b, 255, 100))

typedef unsigned short sixel_pixel_t;

/*
  Structure declarations.
*/
typedef struct sixel_node {
  int
    color,
    left,
    right;

  sixel_pixel_t
    *map;

  struct sixel_node
    *next;
} sixel_node_t;

typedef struct sixel_output {
  Image
    *image;

  int
    active_palette,
    pos,
    save_count,
    save_pixel;

  sixel_node_t
    *node_free,
    *node_top;

  unsigned char
    buffer[MagickMax(SIXEL_OUTPUT_PACKET_SIZE*2,MagickPathExtent)],
    has_8bit_control; /* 0: 7bit terminal, 1: 8bit terminal */
} sixel_output_t;

static int const sixel_default_color_table[] = {
  SIXEL_XRGB(0,  0,  0),   /*  0 Black    */
  SIXEL_XRGB(20, 20, 80),  /*  1 Blue     */
  SIXEL_XRGB(80, 13, 13),  /*  2 Red      */
  SIXEL_XRGB(20, 80, 20),  /*  3 Green    */
  SIXEL_XRGB(80, 20, 80),  /*  4 Magenta  */
  SIXEL_XRGB(20, 80, 80),  /*  5 Cyan     */
  SIXEL_XRGB(80, 80, 20),  /*  6 Yellow   */
  SIXEL_XRGB(53, 53, 53),  /*  7 Gray 50% */
  SIXEL_XRGB(26, 26, 26),  /*  8 Gray 25% */
  SIXEL_XRGB(33, 33, 60),  /*  9 Blue*    */
  SIXEL_XRGB(60, 26, 26),  /* 10 Red*     */
  SIXEL_XRGB(33, 60, 33),  /* 11 Green*   */
  SIXEL_XRGB(60, 33, 60),  /* 12 Magenta* */
  SIXEL_XRGB(33, 60, 60),  /* 13 Cyan*    */
  SIXEL_XRGB(60, 60, 33),  /* 14 Yellow*  */
  SIXEL_XRGB(80, 80, 80),  /* 15 Gray 75% */
};

/*
  Forward declarations.
*/
static MagickBooleanType
  WriteSIXELImage(const ImageInfo *,Image *,ExceptionInfo *);

static int hue_to_rgb(int n1,int n2,int hue)
{
  const int
    HLSMAX=100;

  if (hue < 0)
    hue += HLSMAX;
  if (hue > HLSMAX)
    hue -= HLSMAX;
  if (hue < (HLSMAX/6))
    return(n1 + (((ssize_t) (n2-n1)*hue+(HLSMAX/12))/(HLSMAX/6)));
  if (hue < (HLSMAX/2))
    return(n2);
  if (hue < ((HLSMAX*2)/3))
    return(n1+(((ssize_t) (n2-n1)*(((HLSMAX*2)/3)-hue)+(HLSMAX/12))/(HLSMAX/6)));
  return(n1);
}

static int hls_to_rgb(int hue, int lum, int sat)
{
  const int
    HLSMAX = 100,
    RGBMAX = 255;

  int
    b,
    g,
    magic1,
    magic2,
    r;

  if (sat == 0)
    r=g=b=(lum*(ssize_t) RGBMAX)/HLSMAX;
  else
    {
      if (lum <= (HLSMAX / 2))
        magic2=(int) (((ssize_t) lum*((ssize_t) HLSMAX+sat)+(HLSMAX/2))/HLSMAX);
      else
        magic2=(int) (lum+sat-(((ssize_t) lum*sat)+(HLSMAX/2))/HLSMAX);
      magic1=(int) (2*(ssize_t) lum-magic2);
      b=(hue_to_rgb(magic1,magic2,(ssize_t) hue+(HLSMAX/3))*(ssize_t) RGBMAX+
        (HLSMAX/2))/HLSMAX;
      r=(hue_to_rgb(magic1,magic2,hue)*(ssize_t) RGBMAX+(ssize_t) (HLSMAX/2))/
        HLSMAX;
      g=(hue_to_rgb(magic1,magic2,(ssize_t) hue-(HLSMAX/3))*(ssize_t) RGBMAX+
        (HLSMAX/2))/HLSMAX;
    }
  return(SIXEL_RGB(r,g,b));
}

static unsigned char *get_params(unsigned char *p, int *param, int *len)
{
  int
    n;

  *len=0;
  while (*p != '\0')
  {
    while ((*p == ' ') || (*p == '\t'))
      p++;
    if (isdigit((int) ((unsigned char) *p)))
      {
        for (n = 0; isdigit((int) ((unsigned char) *p)); p++)
        {
          if (n <= (INT_MAX/10))
            n=(int) ((ssize_t) n*10+(*p-'0'));
        }
        if (*len < 10)
          param[(*len)++]=n;
        while (*p == ' ' || *p == '\t')
          p++;
        if (*p == ';')
          p++;
      }
    else if (*p == ';')
      {
        if (*len < 10)
          param[(*len)++]=0;
        p++;
      }
    else
      break;
  }
  return p;
}

/* convert sixel data into indexed pixel bytes and palette data */
static MagickBooleanType sixel_decode(Image *image,unsigned char *p,
  sixel_pixel_t **pixels,size_t *pwidth,size_t *pheight,
  unsigned char **palette,size_t *ncolors,ExceptionInfo *exception)
{
  int
    attributed_pad,
    attributed_pan,
    attributed_ph,
    attributed_pv,
    b,
    background_color_index,
    c,
    color_index,
    dmsx,
    dmsy,
    g,
    i,
    imsx,
    imsy,
    n,
    max_color_index,
    max_x,
    max_y,
    param[10],
    position_x,
    position_y,
    r,
    repeat_count,
    sixel_palet[SIXEL_PALETTE_MAX],
    sixel_vertical_mask,
    x,
    y;

  sixel_pixel_t
    *dmbuf,
    *imbuf;

  size_t
    extent,
    offset;

  extent=strlen((char *) p);
  position_x=position_y=0;
  max_x=max_y=0;
  attributed_pan=2;
  attributed_pad=1;
  attributed_ph=attributed_pv=0;
  repeat_count=1;
  color_index=0;
  background_color_index=0;
  max_color_index=2;
  memset(param,0,sizeof(param));
  imsx=2048;
  imsy=2048;
  if (SetImageExtent(image,(size_t) imsx,(size_t) imsy,exception) == MagickFalse)
    return(MagickFalse);
  imbuf=(sixel_pixel_t *) AcquireQuantumMemory((size_t) imsx,(size_t) imsy*sizeof(sixel_pixel_t));
  if (imbuf == (sixel_pixel_t *) NULL)
    return(MagickFalse);
  for (n = 0; n < 16; n++)
    sixel_palet[n]=sixel_default_color_table[n];
  /* colors 16-231 are a 6x6x6 color cube */
  for (r=0; r < 6; r++)
  {
    for (g=0; g < 6; g++)
    {
      for (b=0; b < 6; b++)
      {
        sixel_palet[n++]=SIXEL_RGB(r*51,g*51,b*51);
      }
    }
  }
  /* colors 232-255 are a grayscale ramp, intentionally leaving out */
  for (i=0; i < 24; i++)
    sixel_palet[n++]=SIXEL_RGB(i*11,i*11,i*11);
  for (; n < SIXEL_PALETTE_MAX; n++)
    sixel_palet[n]=SIXEL_RGB(255,255,255);
  for (i = 0; i < imsx * imsy; i++)
    imbuf[i]=(sixel_pixel_t) background_color_index;
  while (*p != '\0')
  {
    if ((p[0] == '\033' && p[1] == 'P') || (*p == 0x90))
      {
        if (*p == '\033')
          p++;
        p=get_params(++p,param,&n);
        if (*p == 'q')
          {
            p++;
            if (n > 0)
              {
                /* Pn1 */
                switch(param[0])
                {
                  case 0:
                  case 1:
                    attributed_pad = 2;
                    break;
                  case 2:
                    attributed_pad = 5;
                    break;
                  case 3:
                    attributed_pad = 4;
                    break;
                  case 4:
                    attributed_pad = 4;
                    break;
                  case 5:
                    attributed_pad = 3;
                    break;
                  case 6:
                    attributed_pad = 3;
                    break;
                  case 7:
                    attributed_pad = 2;
                    break;
                  case 8:
                    attributed_pad = 2;
                    break;
                  case 9:
                    attributed_pad = 1;
                    break;
                }
              }
            if (n > 2)
              {
                /* Pn3 */
                if (param[2] == 0)
                  param[2]=10;
                attributed_pan=(int) (((ssize_t) attributed_pan*param[2])/10);
                attributed_pad=(int) (((ssize_t) attributed_pad*param[2])/10);
                if (attributed_pan <= 0)
                  attributed_pan=1;
                if (attributed_pad <= 0)
                  attributed_pad=1;
              }
          }
      }
    else if ((p[0] == '\033' && p[1] == '\\') || (*p == 0x9C))
      break;
    else if (*p == '"')
      {
        /* DECGRA Set Raster Attributes " Pan; Pad; Ph; Pv */
        p=get_params(++p,param,&n);
        if (n > 0)
          attributed_pad=param[0];
        if (n > 1)
          attributed_pan=param[1];
        if ((n > 2) && (param[2] > 0))
          attributed_ph=param[2]&0xffff;
        if ((n > 3) && (param[3] > 0))
          attributed_pv=param[3]&0xffff;
        if (attributed_pan <= 0)
          attributed_pan=1;
        if (attributed_pad <= 0)
          attributed_pad=1;
        if ((imsx < attributed_ph) || (imsy < attributed_pv))
          {
            dmsx=imsx > attributed_ph ? imsx : attributed_ph;
            dmsy=imsy > attributed_pv ? imsy : attributed_pv;
            if (SetImageExtent(image,(size_t) dmsx,(size_t) dmsy,exception) == MagickFalse)
              break;
            dmbuf=(sixel_pixel_t *) AcquireQuantumMemory((size_t) dmsx,(size_t)
              dmsy*sizeof(sixel_pixel_t));
            if (dmbuf == (sixel_pixel_t *) NULL)
              {
                imbuf=(sixel_pixel_t *) RelinquishMagickMemory(imbuf);
                return(MagickFalse);
              }
            (void) memset(dmbuf,background_color_index,(size_t) dmsx*(size_t)
              dmsy*sizeof(sixel_pixel_t));
            for (y = 0; y < imsy; ++y)
              (void) memcpy(dmbuf+dmsx*y,imbuf+imsx*y,(size_t) imsx*
                sizeof(sixel_pixel_t));
            imbuf=(sixel_pixel_t *) RelinquishMagickMemory(imbuf);
            imsx=dmsx;
            imsy=dmsy;
            imbuf=dmbuf;
          }
      }
    else if (*p == '!')
      {
        /* DECGRI Graphics Repeat Introducer ! Pn Ch */
        p=get_params(++p,param,&n);
        if ((n > 0) && (param[0] > 0))
          {
            repeat_count=param[0];
            if (repeat_count > (ssize_t) extent)
              break;
          }
      }
    else if (*p == '#')
      {
        /* DECGCI Graphics Color Introducer # Pc; Pu; Px; Py; Pz */
        p=get_params(++p,param,&n);

        if (n > 0)
          {
            if ((color_index = param[0]) < 0)
              color_index=0;
            else if (color_index >= SIXEL_PALETTE_MAX)
              color_index=SIXEL_PALETTE_MAX-1;
          }
        if (n > 4)
          {
            if (param[1] == 1)
              {
                /* HLS */
                if (param[2] > 360)
                  param[2]=360;
                if (param[3] > 100)
                  param[3]=100;
                if (param[4] > 100)
                  param[4]=100;
                sixel_palet[color_index]=hls_to_rgb((int) ((ssize_t) param[2]*
                  100/360),param[3],param[4]);
              }
            else if (param[1] == 2)
              {
                /* RGB */
                if (param[2] > 100)
                  param[2]=100;
                if (param[3] > 100)
                  param[3]=100;
                if (param[4] > 100)
                  param[4]=100;
                sixel_palet[color_index]=SIXEL_XRGB(param[2],param[3],
                  param[4]);
              }
          }
      }
    else if (*p == '$')
      {
        /* DECGCR Graphics Carriage Return */
        p++;
        position_x=0;
        repeat_count=1;
      }
    else if (*p == '-')
      {
        /* DECGNL Graphics Next Line */
        p++;
        position_x=0;
        position_y+=6;
        repeat_count=1;
      }
    else if ((*p >= '?') && (*p <= '\177'))
      {
        if ((imsx < (position_x + repeat_count)) || (imsy < (position_y + 6)))
          {
            int
              nx,
              ny;

            nx=imsx*2;
            ny=imsy*2;

            while ((nx < (position_x + repeat_count)) || (ny < (position_y + 6)))
            {
              nx *= 2;
              ny *= 2;
            }

            dmsx=nx;
            dmsy=ny;
            if (SetImageExtent(image,(size_t) dmsx,(size_t) dmsy,exception) == MagickFalse)
              break;
            dmbuf=(sixel_pixel_t *) AcquireQuantumMemory((size_t) dmsx,(size_t)
              dmsy*sizeof(sixel_pixel_t));
            if (dmbuf == (sixel_pixel_t *) NULL)
              {
                imbuf=(sixel_pixel_t *) RelinquishMagickMemory(imbuf);
                return(MagickFalse);
              }
            (void) memset(dmbuf,background_color_index,(size_t) dmsx*(size_t)
              dmsy*sizeof(sixel_pixel_t));
            for (y = 0; y < imsy; ++y)
              (void) memcpy(dmbuf+dmsx*y,imbuf+imsx*y,(size_t) imsx*
                sizeof(sixel_pixel_t));
            imbuf = (sixel_pixel_t *) RelinquishMagickMemory(imbuf);
            imsx=dmsx;
            imsy=dmsy;
            imbuf=dmbuf;
          }
        if (color_index > max_color_index)
          max_color_index = color_index;
        if ((b = *(p++) - '?') == 0)
          position_x += repeat_count;
        else
          {
            sixel_vertical_mask=0x01;
            if (repeat_count <= 1)
              {
                for (i = 0; i < 6; i++)
                {
                  if ((b & sixel_vertical_mask) != 0)
                    {
                      offset=(size_t) (imsx*((ssize_t) position_y+i)+
                        (ssize_t) position_x);
                      if (offset >= (size_t) (imsx*imsy))
                        {
                          imbuf=(sixel_pixel_t *) RelinquishMagickMemory(imbuf);
                          return(MagickFalse);
                        }
                      imbuf[offset]=(sixel_pixel_t) color_index;
                      if (max_x < position_x)
                          max_x = position_x;
                      if (max_y < (position_y + i))
                          max_y = position_y + i;
                    }
                  sixel_vertical_mask <<= 1;
                }
                position_x += 1;
              }
            else  /* repeat_count > 1 */
              {
                for (i = 0; i < 6; i++)
                {
                  if ((b & sixel_vertical_mask) != 0)
                    {
                      c=sixel_vertical_mask << 1;
                      for (n=1; (i+n) < 6; n++)
                      {
                        if ((b & c) == 0)
                          break;
                        c <<= 1;
                      }
                      for (y = position_y + i; y < position_y + i + n; ++y)
                      {
                        offset=(size_t) ((ssize_t) imsx*y+(ssize_t) position_x);
                        if ((offset+(size_t) repeat_count) >= (size_t) (imsx*imsy))
                          {
                            imbuf=(sixel_pixel_t *) RelinquishMagickMemory(imbuf);
                            return(MagickFalse);
                          }
                        for (x = 0; x < repeat_count; x++)
                          imbuf[(int) offset+x]=(sixel_pixel_t) color_index;
                      }
                      if (max_x < (position_x+repeat_count-1))
                        max_x = position_x+repeat_count-1;
                      if (max_y < (position_y+i+n-1))
                        max_y = position_y+i+n-1;
                      i+=(n-1);
                      sixel_vertical_mask <<= (n-1);
                    }
                  sixel_vertical_mask <<= 1;
                }
                position_x += repeat_count;
              }
          }
          repeat_count = 1;
      }
    else
      p++;
  }
  if (++max_x < attributed_ph)
    max_x = attributed_ph;
  if (++max_y < attributed_pv)
    max_y = attributed_pv;
  if ((imsx > max_x) || (imsy > max_y))
    {
      dmsx=max_x;
      dmsy=max_y;
      if (SetImageExtent(image,(size_t) dmsx,(size_t) dmsy,exception) == MagickFalse)
        {
          imbuf=(sixel_pixel_t *) RelinquishMagickMemory(imbuf);
          return(MagickFalse);
        }
      dmbuf=(sixel_pixel_t *) AcquireQuantumMemory((size_t) dmsx,(size_t) dmsy*
        sizeof(sixel_pixel_t));
      if (dmbuf == (sixel_pixel_t *) NULL)
        {
          imbuf=(sixel_pixel_t *) RelinquishMagickMemory(imbuf);
          return(MagickFalse);
        }
      for (y=0; y < dmsy; ++y)
        (void) memcpy(dmbuf+dmsx*y,imbuf+imsx*y,(size_t) dmsx*
          sizeof(sixel_pixel_t));
      imbuf=(sixel_pixel_t *) RelinquishMagickMemory(imbuf);
      imsx=dmsx;
      imsy=dmsy;
      imbuf=dmbuf;
    }

  *pixels=imbuf;
  *pwidth=(size_t) imsx;
  *pheight=(size_t) imsy;
  *ncolors=(size_t) max_color_index+1;
  *palette=(unsigned char *) AcquireQuantumMemory(*ncolors,4);
  if (*palette == (unsigned char *) NULL)
    return(MagickFalse);
  for (n = 0; n < (ssize_t) *ncolors; ++n)
  {
    (*palette)[n*4+0]=sixel_palet[n] >> 16 & 0xff;
    (*palette)[n*4+1]=sixel_palet[n] >> 8 & 0xff;
    (*palette)[n*4+2]=sixel_palet[n] & 0xff;
    (*palette)[n*4+3]=0xff;
  }
  return(MagickTrue);
}

static sixel_output_t *sixel_output_create(Image *image)
{
  sixel_output_t
    *output;

  output=(sixel_output_t *) AcquireMagickMemory(sizeof(sixel_output_t));
  if (output == (sixel_output_t *) NULL)
    return((sixel_output_t *) NULL);
  output->has_8bit_control=0;
  output->save_pixel=0;
  output->save_count=0;
  output->active_palette=(-1);
  output->node_top=(sixel_node_t *) NULL;
  output->node_free=(sixel_node_t *) NULL;
  output->image=image;
  output->pos=0;
  return(output);
}

static void sixel_advance(sixel_output_t *context, int nwrite)
{
  if ((context->pos += nwrite) >= SIXEL_OUTPUT_PACKET_SIZE)
    {
      WriteBlob(context->image,SIXEL_OUTPUT_PACKET_SIZE,context->buffer);
      memmove(context->buffer,
              context->buffer + SIXEL_OUTPUT_PACKET_SIZE,(size_t)
              (context->pos -= SIXEL_OUTPUT_PACKET_SIZE));
    }
}

static int sixel_put_flash(sixel_output_t *const context)
{
  int
    n,
    nwrite;

  if (context->save_count > 3)
    {
      /* DECGRI Graphics Repeat Introducer ! Pn Ch */
      nwrite=(int) FormatLocaleString((char *) context->buffer+context->pos,
        sizeof(context->buffer),"!%d%c",context->save_count,
        (char) context->save_pixel);
      if (nwrite <= 0)
        return(-1);
      sixel_advance(context,nwrite);
    }
  else
    {
      for (n = 0; n < context->save_count; n++)
      {
        context->buffer[context->pos]=(unsigned char)context->save_pixel;
        sixel_advance(context,1);
      }
    }
  context->save_pixel=0;
  context->save_count=0;
  return(0);
}

static void sixel_put_pixel(sixel_output_t *const context,int pix)
{
  if ((pix < 0) || (pix > '?'))
    pix=0;
  pix+='?';
  if (pix == context->save_pixel)
    context->save_count++;
  else
    {
      sixel_put_flash(context);
      context->save_pixel=pix;
      context->save_count=1;
    }
}

static void sixel_node_del(sixel_output_t *const context,sixel_node_t *np)
{
  sixel_node_t
    *tp;

  if ((tp = context->node_top) == np)
    context->node_top = np->next;
  else
    {
      while (tp->next != (struct sixel_node *) NULL)
      {
        if (tp->next == np)
          {
            tp->next=np->next;
            break;
          }
        tp=tp->next;
      }
    }
  np->next=context->node_free;
  context->node_free=np;
}

static int sixel_put_node(sixel_output_t *const context,int x,sixel_node_t *np,
  int ncolors,int keycolor)
{
  int
    nwrite;

  if ((ncolors != 2) || (keycolor == -1))
    {
      /* designate palette index */
      if (context->active_palette != np->color)
        {
          nwrite=(int) FormatLocaleString((char *) context->buffer+context->pos,
            sizeof(context->buffer),"#%d",np->color);
          sixel_advance(context,nwrite);
          context->active_palette=np->color;
        }
    }
  for (; x < np->left; x++)
    sixel_put_pixel(context,0);
  for (; x < np->right; x++)
    sixel_put_pixel(context,np->map[x]);
  sixel_put_flash(context);
  return(x);
}

static MagickBooleanType sixel_encode_impl(sixel_pixel_t *pixels,size_t width,
  size_t height,unsigned char *palette,size_t ncolors,int keycolor,
  sixel_output_t *context)
{
#define RelinquishNodesAndMap \
  while ((np = context->node_free) != NULL) \
  { \
    context->node_free=np->next; \
    np=(sixel_node_t *) RelinquishMagickMemory(np); \
  } \
  map=(sixel_pixel_t *) RelinquishMagickMemory(map)

  int
    c,
    i,
    left,
    pix,
    n,
    nwrite,
    right,
    x,
    y;

  sixel_pixel_t
    *map;

  sixel_node_t
    *np,
    top,
    *tp;

  size_t
    len;

  context->pos = 0;
  if (ncolors < 1)
    return(MagickFalse);
  len=ncolors*width;
  context->active_palette=(-1);
  map=(sixel_pixel_t *) AcquireQuantumMemory(len,sizeof(sixel_pixel_t));
  if (map == (sixel_pixel_t *) NULL)
    return (MagickFalse);
  (void) memset(map,0,len*sizeof(sixel_pixel_t));
  if (context->has_8bit_control)
    nwrite=(int) FormatLocaleString((char *) context->buffer,sizeof(context->buffer),
      "\x90" "0;0;0" "q");
  else
    nwrite=(int) FormatLocaleString((char *) context->buffer,sizeof(context->buffer),
      "\x1bP" "0;0;0" "q");
  if (nwrite <= 0)
    return(MagickFalse);
  sixel_advance(context,nwrite);
  nwrite=(int) FormatLocaleString((char *) context->buffer+context->pos,
    sizeof(context->buffer),"\"1;1;%d;%d",(int) width,(int) height);
  if (nwrite <= 0)
    {
      RelinquishNodesAndMap;
      return(MagickFalse);
    }
  sixel_advance(context,nwrite);
  if ((ncolors != 2) || (keycolor == -1))
    for (n = 0; n < (ssize_t) ncolors; n++)
    {
        /* DECGCI Graphics Color Introducer  # Pc ; Pu; Px; Py; Pz */
        nwrite=(int) FormatLocaleString((char *) context->buffer+context->pos,
          sizeof(context->buffer),"#%d;2;%d;%d;%d",n,(palette[n*3+0]*100+127)/
          255,(palette[n*3+1]*100+127)/255,(palette[n*3+2]*100+127)/255);
        if (nwrite <= 0)
          {
            RelinquishNodesAndMap;
            return(MagickFalse);
          }
        sixel_advance(context,nwrite);
        if (nwrite <= 0)
          {
            RelinquishNodesAndMap;
            return (MagickFalse);
          }
    }
  for (y=i=0; y < (int) height; y++)
  {
    for (x=0; x < (int) width; x++)
    {
      pix=pixels[y*(ssize_t) width+x];
      if ((pix >= 0) && (pix < (int) ncolors) && (pix != keycolor))
        map[pix*(ssize_t) width+x]|=(1 << i);
    }
    if (++i < 6 && (y + 1) < (int) height)
      continue;
    for (c=0; c < (int) ncolors; c++)
    {
      for (left=0; left < (int) width; left++)
      {
        if (*(map+c*(ssize_t) width+left) == 0)
          continue;
        for (right=left+1; right < (int) width; right++)
        {
          if (*(map+c*(ssize_t) width+right) != 0)
            continue;
          for (n = 1; (right+n) < (int) width; n++)
          {
            if (*(map+c*(ssize_t) width+right+n) != 0)
              break;
          }
          if ((n >= 10) || (right+n >= (int) width))
            break;
          right=right+n-1;
        }

        if ((np = context->node_free) != (sixel_node_t *) NULL)
          context->node_free=np->next;
        else
          if ((np = (sixel_node_t *) AcquireMagickMemory(
                sizeof(sixel_node_t))) == (sixel_node_t *) NULL)
            {
              RelinquishNodesAndMap;
              return(MagickFalse);
            }
        np->color=c;
        np->left=left;
        np->right=right;
        np->map=map+c*(ssize_t) width;
        top.next=context->node_top;
        tp=&top;
        while (tp->next != (struct sixel_node *) NULL)
        {
          if (np->left < tp->next->left)
            break;
          if ((np->left == tp->next->left) && (np->right > tp->next->right))
              break;
          tp=tp->next;
        }
        np->next=tp->next;
        tp->next=np;
        context->node_top=top.next;
        left=right-1;
      }
    }
    for (x = 0; (np = context->node_top) != (sixel_node_t *) NULL;)
    {
      if (x > np->left)
        {
          /* DECGCR Graphics Carriage Return */
          context->buffer[context->pos]='$';
          sixel_advance(context,1);
          x=0;
        }
      x=sixel_put_node(context,x,np,(int) ncolors,keycolor);
      sixel_node_del(context,np);
      np=context->node_top;
      while (np != (sixel_node_t *) NULL)
      {
        if (np->left < x)
          {
            np=np->next;
            continue;
          }
          x=sixel_put_node(context,x,np,(int) ncolors,keycolor);
          sixel_node_del(context,np);
          np=context->node_top;
      }
    }
    /* DECGNL Graphics Next Line */
    context->buffer[context->pos]='-';
    sixel_advance(context,1);
    if (nwrite <= 0)
      {
        RelinquishNodesAndMap;
        return(MagickFalse);
      }
    i=0;
    (void) memset(map,0,len*sizeof(sixel_pixel_t));
  }
  if (context->has_8bit_control)
    {
      context->buffer[context->pos]=0x9c;
      sixel_advance(context,1);
    }
  else
    {
      context->buffer[context->pos]=0x1b;
      context->buffer[context->pos+1]='\\';
      sixel_advance(context,2);
    }
  if (nwrite <= 0)
    {
      RelinquishNodesAndMap;
      return(MagickFalse);
    }
  /* flush buffer */
  if (context->pos > 0)
    WriteBlob(context->image,(size_t) context->pos,context->buffer);
  RelinquishNodesAndMap;
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s S I X E L                                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsSIXEL() returns MagickTrue if the image format type, identified by the
%  magick string, is SIXEL.
%
%  The format of the IsSIXEL method is:
%
%      MagickBooleanType IsSIXEL(const unsigned char *magick,
%        const size_t length)
%
%  A description of each parameter follows:
%
%    o magick: compare image format pattern against these bytes. or
%      blob.
%
%    o length: Specifies the length of the magick string.
%
*/
static MagickBooleanType IsSIXEL(const unsigned char *magick,
  const size_t length)
{
  const unsigned char
    *end = magick + length;

  if (length < 3)
    return(MagickFalse);

  if (*magick == 0x90 || (*magick == 0x1b && *++magick == 'P')) {
    while (++magick != end) {
      if (*magick == 'q')
        return(MagickTrue);
      if (!(*magick >= '0' && *magick <= '9') && *magick != ';')
        return(MagickFalse);
    }
  }
  return(MagickFalse);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d S I X E L I m a g e                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadSIXELImage() reads an X11 pixmap image file and returns it.  It
%  allocates the memory necessary for the new Image structure and returns a
%  pointer to the new image.
%
%  The format of the ReadSIXELImage method is:
%
%      Image *ReadSIXELImage(const ImageInfo *image_info,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadSIXELImage(const ImageInfo *image_info,
  ExceptionInfo *exception)
{
  char
    *p,
    *sixel_buffer;

  Image
    *image;

  MagickBooleanType
    status;

  Quantum
    *q;

  size_t
    length;

  sixel_pixel_t
    *sixel_pixels;

  ssize_t
    i,
    j,
    y;

  unsigned char
    *sixel_palette;

  /*
    Open image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",
      image_info->filename);
  image=AcquireImage(image_info,exception);
  status=OpenBlob(image_info,image,ReadBinaryBlobMode,exception);
  if (status == MagickFalse)
    {
      image=DestroyImageList(image);
      return((Image *) NULL);
    }
  /*
    Read SIXEL file.
  */
  length=MagickPathExtent;
  sixel_buffer=(char *) AcquireQuantumMemory((size_t) length+MagickPathExtent,
    sizeof(*sixel_buffer));
  p=sixel_buffer;
  if (sixel_buffer != (char *) NULL)
    while (ReadBlobString(image,p) != (char *) NULL)
    {
      ssize_t
       offset;

      if ((*p == '#') && ((p == sixel_buffer) || (*(p-1) == '\n')))
        continue;
      if ((*p == '}') && (*(p+1) == ';'))
        break;
      p+=(ptrdiff_t) strlen(p);
      offset=p-sixel_buffer;
      if ((size_t) (offset+MagickPathExtent+1) < length)
        continue;
      length<<=1;
      sixel_buffer=(char *) ResizeQuantumMemory(sixel_buffer,length+
        MagickPathExtent+1,sizeof(*sixel_buffer));
      if (sixel_buffer == (char *) NULL)
        break;
      p=sixel_buffer+offset;
    }
  if (sixel_buffer == (char *) NULL)
    ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
  sixel_buffer[length]='\0';
  /*
    Decode SIXEL.
  */
  sixel_pixels=(sixel_pixel_t *) NULL;
  status=sixel_decode(image,(unsigned char *) sixel_buffer,&sixel_pixels,
    &image->columns,&image->rows,&sixel_palette,&image->colors,
    exception);
  if (status == MagickFalse)
    {
      sixel_buffer=(char *) RelinquishMagickMemory(sixel_buffer);
      if (sixel_pixels != (sixel_pixel_t *) NULL)
        sixel_pixels=(sixel_pixel_t *) RelinquishMagickMemory(sixel_pixels);
      ThrowReaderException(CorruptImageError,"CorruptImage");
    }
  sixel_buffer=(char *) RelinquishMagickMemory(sixel_buffer);
  image->depth=24;
  image->storage_class=PseudoClass;
  status=SetImageExtent(image,image->columns,image->rows,exception);
  if (status == MagickFalse)
    {
      sixel_pixels=(sixel_pixel_t *) RelinquishMagickMemory(sixel_pixels);
      sixel_palette=(unsigned char *) RelinquishMagickMemory(sixel_palette);
      return(DestroyImageList(image));
    }
  if (AcquireImageColormap(image,image->colors, exception) == MagickFalse)
    {
      sixel_pixels=(sixel_pixel_t *) RelinquishMagickMemory(sixel_pixels);
      sixel_palette=(unsigned char *) RelinquishMagickMemory(sixel_palette);
      ThrowReaderException(ResourceLimitError,"MemoryAllocationFailed");
    }
  for (i = 0; i < (ssize_t) image->colors; ++i)
  {
    image->colormap[i].red=ScaleCharToQuantum(sixel_palette[i * 4 + 0]);
    image->colormap[i].green=ScaleCharToQuantum(sixel_palette[i * 4 + 1]);
    image->colormap[i].blue=ScaleCharToQuantum(sixel_palette[i * 4 + 2]);
  }
  j=0;
  if (image_info->ping == MagickFalse)
    {
      /*
        Read image pixels.
      */
      for (y=0; y < (ssize_t) image->rows; y++)
      {
        ssize_t
          x;

        q=QueueAuthenticPixels(image,0,y,image->columns,1,exception);
        if (q == (Quantum *) NULL)
          break;
        for (x=0; x < (ssize_t) image->columns; x++)
        {
          j=(ssize_t) sixel_pixels[y*(ssize_t) image->columns+x];
          j=ConstrainColormapIndex(image,j,exception);
          SetPixelIndex(image,(Quantum) j,q);
          SetPixelRed(image,(Quantum) image->colormap[j].red,q);
          SetPixelGreen(image,(Quantum) image->colormap[j].green,q);
          SetPixelBlue(image,(Quantum) image->colormap[j].blue,q);
          q+=(ptrdiff_t) GetPixelChannels(image);
        }
        if (SyncAuthenticPixels(image,exception) == MagickFalse)
          break;
      }
      if (y < (ssize_t) image->rows)
        {
          sixel_pixels=(sixel_pixel_t *) RelinquishMagickMemory(sixel_pixels);
          sixel_palette=(unsigned char *) RelinquishMagickMemory(sixel_palette);
          ThrowReaderException(CorruptImageError,"NotEnoughPixelData");
        }
    }
  /*
    Relinquish resources.
  */
  sixel_pixels=(sixel_pixel_t *) RelinquishMagickMemory(sixel_pixels);
  sixel_palette=(unsigned char *) RelinquishMagickMemory(sixel_palette);
  if (CloseBlob(image) == MagickFalse)
    status=MagickFalse;
  if (status == MagickFalse)
    return(DestroyImageList(image));
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r S I X E L I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterSIXELImage() adds attributes for the SIXEL image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterSIXELImage method is:
%
%      size_t RegisterSIXELImage(void)
%
*/
ModuleExport size_t RegisterSIXELImage(void)
{
  MagickInfo
    *entry;

  entry=AcquireMagickInfo("SIXEL","SIXEL","DEC SIXEL Graphics Format");
  entry->decoder=(DecodeImageHandler *) ReadSIXELImage;
  entry->encoder=(EncodeImageHandler *) WriteSIXELImage;
  entry->magick=(IsImageFormatHandler *) IsSIXEL;
  entry->flags^=CoderAdjoinFlag;
  (void) RegisterMagickInfo(entry);
  entry=AcquireMagickInfo("SIXEL","SIX","DEC SIXEL Graphics Format");
  entry->decoder=(DecodeImageHandler *) ReadSIXELImage;
  entry->encoder=(EncodeImageHandler *) WriteSIXELImage;
  entry->magick=(IsImageFormatHandler *) IsSIXEL;
  entry->flags^=CoderAdjoinFlag;
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r S I X E L I m a g e                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterSIXELImage() removes format registrations made by the
%  SIXEL module from the list of supported formats.
%
%  The format of the UnregisterSIXELImage method is:
%
%      UnregisterSIXELImage(void)
%
*/
ModuleExport void UnregisterSIXELImage(void)
{
  (void) UnregisterMagickInfo("SIXEL");
  (void) UnregisterMagickInfo("SIX");
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   W r i t e S I X E L I m a g e                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  WriteSIXELImage() writes an image to a file in the X pixmap format.
%
%  The format of the WriteSIXELImage method is:
%
%      MagickBooleanType WriteSIXELImage(const ImageInfo *image_info,
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
static MagickBooleanType WriteSIXELImage(const ImageInfo *image_info,
  Image *image,ExceptionInfo *exception)
{
  MagickBooleanType
    status;

  const Quantum
    *q;

  ssize_t
    i,
    x;

  ssize_t
    opacity,
    y;

  sixel_output_t
    *output;

  unsigned char
    sixel_palette[SIXEL_PALETTE_MAX*3];

  sixel_pixel_t
    *sixel_pixels;

  /*
    Open output image file.
  */
  assert(image_info != (const ImageInfo *) NULL);
  assert(image_info->signature == MagickCoreSignature);
  assert(image != (Image *) NULL);
  assert(image->signature == MagickCoreSignature);
  if (IsEventLogging() != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  status=OpenBlob(image_info,image,WriteBinaryBlobMode,exception);
  if (status == MagickFalse)
    return(status);
  if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
    (void) TransformImageColorspace(image,sRGBColorspace,exception);
  opacity=(-1);
  if ((image->alpha_trait & BlendPixelTrait) == 0)
    {
      if ((image->storage_class == DirectClass) || (image->colors > SIXEL_PALETTE_MAX))
        (void) SetImageType(image,PaletteType,exception);
    }
  else
    {
      MagickRealType
        alpha,
        beta;

      /*
        Identify transparent colormap index.
      */
      if ((image->storage_class == DirectClass) ||
          (image->colors > SIXEL_PALETTE_MAX))
        (void) SetImageType(image,PaletteBilevelAlphaType,exception);
      for (i=0; i < (ssize_t) image->colors; i++)
        if (image->colormap[i].alpha != (double) OpaqueAlpha)
          {
            if (opacity < 0)
              {
                opacity=i;
                continue;
              }
            alpha=image->colormap[i].alpha;
            beta=image->colormap[opacity].alpha;
            if (alpha < beta)
              opacity=i;
          }
      if (opacity == -1)
        {
          (void) SetImageType(image,PaletteBilevelAlphaType,exception);
          for (i=0; i < (ssize_t) image->colors; i++)
            if (image->colormap[i].alpha != (double) OpaqueAlpha)
              {
                if (opacity < 0)
                  {
                    opacity=i;
                    continue;
                  }
                alpha=image->colormap[i].alpha;
                beta=image->colormap[opacity].alpha;
                if (alpha < beta)
                  opacity=i;
              }
        }
      if (opacity >= 0)
        {
          image->colormap[opacity].red=image->transparent_color.red;
          image->colormap[opacity].green=image->transparent_color.green;
          image->colormap[opacity].blue=image->transparent_color.blue;
        }
    }
  /*
    SIXEL header.
  */
  for (i=0; i < (ssize_t) image->colors; i++)
  {
    sixel_palette[3*i+0]=ScaleQuantumToChar((Quantum) image->colormap[i].red);
    sixel_palette[3*i+1]=ScaleQuantumToChar((Quantum) image->colormap[i].green);
    sixel_palette[3*i+2]=ScaleQuantumToChar((Quantum) image->colormap[i].blue);
  }
  /*
    Define SIXEL pixels.
  */
  output=sixel_output_create(image);
  if (output == (sixel_output_t *) NULL)
    ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
  sixel_pixels=(sixel_pixel_t *) AcquireQuantumMemory(image->columns,
    image->rows*sizeof(sixel_pixel_t));
  if (sixel_pixels == (sixel_pixel_t *) NULL)
    {
      output=(sixel_output_t *) RelinquishMagickMemory(output);
      ThrowWriterException(ResourceLimitError,"MemoryAllocationFailed");
    }
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    q=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (q == (Quantum *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      sixel_pixels[y*(ssize_t) image->columns+x]=((sixel_pixel_t)
        GetPixelIndex(image,q));
      q+=(ptrdiff_t) GetPixelChannels(image);
    }
  }
  status=sixel_encode_impl(sixel_pixels,image->columns,image->rows,
    sixel_palette,image->colors,-1,output);
  sixel_pixels=(sixel_pixel_t *) RelinquishMagickMemory(sixel_pixels);
  output=(sixel_output_t *) RelinquishMagickMemory(output);
  if (CloseBlob(image) == MagickFalse)
    status=MagickFalse;
  return(status);
}
