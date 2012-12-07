/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                            SSSSS   CCCC  RRRR                               %
%                            SS     C      R   R                              %
%                             SSS   C      RRRR                               %
%                               SS  C      R R                                %
%                            SSSSS   CCCC  R  R                               %
%                                                                             %
%                                                                             %
%                      Read ZX-Spectrum SCREEN$ Format                        %
%                                                                             %
%                              Software Design                                %
%                              Catalin Mihaila                                %
%                               October 2003                                  %
%                                                                             %
%                                                                             %
%  Copyright 1999-2013 ImageMagick Studio LLC, a non-profit organization      %
%  dedicated to making software imaging solutions freely available.           %
%                                                                             %
%  You may not use this file except in compliance with the License.  You may  %
%  obtain a copy of the License at                                            %
%                                                                             %
%    http://www.imagemagick.org/script/license.php                            %
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
#include "magick/studio.h"
#include "magick/blob.h"
#include "magick/blob-private.h"
#include "magick/cache.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/image.h"
#include "magick/image-private.h"
#include "magick/list.h"
#include "magick/magick.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/pixel-accessor.h"
#include "magick/quantum-private.h"
#include "magick/static.h"
#include "magick/string_.h"
#include "magick/module.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e a d S C R I m a g e                                                   %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ReadSCRImage() reads a Scitex image file and returns it.  It allocates
%  the memory necessary for the new Image structure and returns a pointer to
%  the new image.
%
%  The format of the ReadSCRImage method is:
%
%      Image *ReadSCRImage(const ImageInfo *image_info,ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image_info: the image info.
%
%    o exception: return any errors or warnings in this structure.
%
*/
static Image *ReadSCRImage(const ImageInfo *image_info,ExceptionInfo *exception)
{
    char zxscr[6144];
    char zxattr[768];
    int octetnr;
    int octetline;
    int zoneline;
    int zonenr;
    int octet_val;
    int attr_nr;
    int pix;
    int piy;
    int binar[8];
    int attrbin[8];
    int *pbin;
    int *abin;
    int z;
    int one_nr;
    int ink;
    int paper;
    int bright;

  unsigned char colour_palette[] = {
      0,  0,  0,
      0,  0,192,
    192,  0,  0,
    192,  0,192,
      0,192,  0,
      0,192,192,
    192,192,  0,
    192,192,192,
      0,  0,  0,
      0,  0,255,
    255,  0,  0,
    255,  0,255,
      0,255,  0,
      0,255,255,
    255,255,  0,
    255,255,255
  };

  Image
    *image;

  MagickBooleanType
    status;

  register PixelPacket
    *q;

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
  image->columns = 256;
  image->rows = 192;
  count=ReadBlob(image,6144,(unsigned char *) zxscr);
  (void) count;
  count=ReadBlob(image,768,(unsigned char *) zxattr);
  for(zonenr=0;zonenr<3;zonenr++)
  {
      for(zoneline=0;zoneline<8;zoneline++)
        {
        for(octetline=0;octetline<8;octetline++)
      {
          for(octetnr=(zoneline*32);octetnr<((zoneline*32)+32);octetnr++)
            {
            octet_val = zxscr[octetnr+(256*octetline)+(zonenr*2048)];
            attr_nr = zxattr[octetnr+(256*zonenr)];

            pix = (((8*octetnr)-(256*zoneline)));
            piy = ((octetline+(8*zoneline)+(zonenr*64)));

            pbin = binar;
            abin = attrbin;

            one_nr=1;

            for(z=0;z<8;z++)
          {
              if(octet_val&one_nr)
            {
                *pbin = 1;
            } else {
                *pbin = 0;
            }
              one_nr=one_nr*2;
              pbin++;
          }

            one_nr = 1;

            for(z=0;z<8;z++)
          {
              if(attr_nr&one_nr)
            {
                *abin = 1;
            } else {
                *abin = 0;
            }
              one_nr=one_nr*2;
              abin++;
          }

            ink = (attrbin[0]+(2*attrbin[1])+(4*attrbin[2]));
            paper = (attrbin[3]+(2*attrbin[4])+(4*attrbin[5]));
            bright = attrbin[6];

            if(bright) { ink=ink+8; paper=paper+8; }

            for(z=7;z>-1;z--)
          {
              q=QueueAuthenticPixels(image,pix,piy,1,1,exception);

              if(binar[z])
            {
                SetPixelRed(q,ScaleCharToQuantum(
                  colour_palette[3*ink]));
                SetPixelGreen(q,ScaleCharToQuantum(
                  colour_palette[1+(3*ink)]));
                SetPixelBlue(q,ScaleCharToQuantum(
                  colour_palette[2+(3*ink)]));
            } else {
                SetPixelRed(q,ScaleCharToQuantum(
                  colour_palette[3*paper]));
                SetPixelGreen(q,ScaleCharToQuantum(
                  colour_palette[1+(3*paper)]));
                SetPixelBlue(q,ScaleCharToQuantum(
                  colour_palette[2+(3*paper)]));
            }

              pix++;
          }
        }
      }
    }
  }
  (void) CloseBlob(image);
  return(GetFirstImageInList(image));
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   R e g i s t e r S C R I m a g e                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  RegisterSCRImage() adds attributes for the SCR image format to
%  the list of supported formats.  The attributes include the image format
%  tag, a method to read and/or write the format, whether the format
%  supports the saving of more than one frame to the same file or blob,
%  whether the format supports native in-memory I/O, and a brief
%  description of the format.
%
%  The format of the RegisterSCRImage method is:
%
%      size_t RegisterSCRImage(void)
%
*/
ModuleExport size_t RegisterSCRImage(void)
{
  MagickInfo
    *entry;

  entry=SetMagickInfo("SCR");
  entry->decoder=(DecodeImageHandler *) ReadSCRImage;
  entry->adjoin=MagickFalse;
  entry->description=ConstantString("ZX-Spectrum SCREEN$");
  entry->module=ConstantString("SCR");
  (void) RegisterMagickInfo(entry);
  return(MagickImageCoderSignature);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   U n r e g i s t e r S C R I m a g e                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  UnregisterSCRImage() removes format registrations made by the
%  SCR module from the list of supported formats.
%
%  The format of the UnregisterSCRImage method is:
%
%      UnregisterSCRImage(void)
%
*/
ModuleExport void UnregisterSCRImage(void)
{
  (void) UnregisterMagickInfo("SCR");
}
