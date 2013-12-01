/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%                                                                             %
%                                 N   N  TTTTT                                %
%                                 NN  N    T                                  %
%                                 N N N    T                                  %
%                                 N  NN    T                                  %
%                                 N   N    T                                  %
%                                                                             %
%                                                                             %
%                   Windows NT Feature Methods for MagickCore                 %
%                                                                             %
%                               Software Design                               %
%                                    Cristy                                   %
%                                December 1996                                %
%                                                                             %
%                                                                             %
%  Copyright 1999-2014 ImageMagick Studio LLC, a non-profit organization      %
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
#if defined(MAGICKCORE_WINDOWS_SUPPORT) || defined(__CYGWIN__)
#define WIN32_LEAN_AND_MEAN
#define VC_EXTRALEAN
#include <windows.h>
#include "magick/cache.h"
#include "magick/colorspace.h"
#include "magick/colorspace-private.h"
#include "magick/draw.h"
#include "magick/exception.h"
#include "magick/exception-private.h"
#include "magick/image-private.h"
#include "magick/memory_.h"
#include "magick/monitor.h"
#include "magick/monitor-private.h"
#include "magick/quantum.h"
#include "magick/string_.h"
#include "magick/token.h"
#include "magick/splay-tree.h"
#include "magick/utility.h"
#include "magick/nt-feature.h"

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   C r o p I m a g e T o H B i t m a p                                       %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  CropImageToHBITMAP() extracts a specified region of the image and returns
%  it as a Windows HBITMAP. While the same functionality can be accomplished by
%  invoking CropImage() followed by ImageToHBITMAP(), this method is more
%  efficient since it copies pixels directly to the HBITMAP.
%
%  The format of the CropImageToHBITMAP method is:
%
%      HBITMAP CropImageToHBITMAP(Image* image,const RectangleInfo *geometry,
%        ExceptionInfo *exception)
%
%  A description of each parameter follows:
%
%    o image: the image.
%
%    o geometry: Define the region of the image to crop with members
%      x, y, width, and height.
%
%    o exception: return any errors or warnings in this structure.
%
*/
MagickExport void *CropImageToHBITMAP(Image *image,
  const RectangleInfo *geometry,ExceptionInfo *exception)
{
#define CropImageTag  "Crop/Image"

  BITMAP
    bitmap;

  HBITMAP
    bitmapH;

  HANDLE
    bitmap_bitsH;

  MagickBooleanType
    proceed;

  RectangleInfo
    page;

  register const PixelPacket
    *p;

  register RGBQUAD
    *q;

  RGBQUAD
    *bitmap_bits;

  ssize_t
    y;

  /*
    Check crop geometry.
  */
  assert(image != (const Image *) NULL);
  assert(image->signature == MagickSignature);
  if (image->debug != MagickFalse)
    (void) LogMagickEvent(TraceEvent,GetMagickModule(),"%s",image->filename);
  assert(geometry != (const RectangleInfo *) NULL);
  assert(exception != (ExceptionInfo *) NULL);
  assert(exception->signature == MagickSignature);
  if (((geometry->x+(ssize_t) geometry->width) < 0) ||
      ((geometry->y+(ssize_t) geometry->height) < 0) ||
      (geometry->x >= (ssize_t) image->columns) ||
      (geometry->y >= (ssize_t) image->rows))
    ThrowImageException(OptionError,"GeometryDoesNotContainImage");
  page=(*geometry);
  if ((page.x+(ssize_t) page.width) > (ssize_t) image->columns)
    page.width=image->columns-page.x;
  if ((page.y+(ssize_t) page.height) > (ssize_t) image->rows)
    page.height=image->rows-page.y;
  if (page.x < 0)
    {
      page.width+=page.x;
      page.x=0;
    }
  if (page.y < 0)
    {
      page.height+=page.y;
      page.y=0;
    }

  if ((page.width == 0) || (page.height == 0))
    ThrowImageException(OptionError,"GeometryDimensionsAreZero");
  /*
    Initialize crop image attributes.
  */
  bitmap.bmType         = 0;
  bitmap.bmWidth        = (LONG) page.width;
  bitmap.bmHeight       = (LONG) page.height;
  bitmap.bmWidthBytes   = bitmap.bmWidth * 4;
  bitmap.bmPlanes       = 1;
  bitmap.bmBitsPixel    = 32;
  bitmap.bmBits         = NULL;

  bitmap_bitsH=(HANDLE) GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE,page.width*
    page.height*bitmap.bmBitsPixel);
  if (bitmap_bitsH == NULL)
    return(NULL);
  bitmap_bits=(RGBQUAD *) GlobalLock((HGLOBAL) bitmap_bitsH);
  if ( bitmap.bmBits == NULL )
    bitmap.bmBits = bitmap_bits;
  if (IssRGBCompatibleColorspace(image->colorspace) == MagickFalse)
    (void) SetImageColorspace(image,sRGBColorspace);
  /*
    Extract crop image.
  */
  q=bitmap_bits;
  for (y=0; y < (ssize_t) page.height; y++)
  {
    p=GetVirtualPixels(image,page.x,page.y+y,page.width,1,exception);
    if (p == (const PixelPacket *) NULL)
      break;

#if MAGICKCORE_QUANTUM_DEPTH == 8
      /* Form of PixelPacket is identical to RGBQUAD when MAGICKCORE_QUANTUM_DEPTH==8 */
      CopyMagickMemory((void*)q,(const void*)p,page.width*sizeof(PixelPacket));
      q += page.width;

#else  /* 16 or 32 bit Quantum */
      {
        ssize_t
          x;

        /* Transfer pixels, scaling to Quantum */
        for( x=(ssize_t) page.width ; x> 0 ; x-- )
          {
            q->rgbRed = ScaleQuantumToChar(GetPixelRed(p));
            q->rgbGreen = ScaleQuantumToChar(GetPixelGreen(p));
            q->rgbBlue = ScaleQuantumToChar(GetPixelBlue(p));
            q->rgbReserved = 0;
            ++q;
            ++p;
          }
      }
#endif
    proceed=SetImageProgress(image,CropImageTag,y,page.height);
    if (proceed == MagickFalse)
      break;
  }
  if (y < (ssize_t) page.height)
    {
      GlobalUnlock((HGLOBAL) bitmap_bitsH);
      GlobalFree((HGLOBAL) bitmap_bitsH);
      return((void *) NULL);
    }
  bitmap.bmBits=bitmap_bits;
  bitmapH=CreateBitmapIndirect(&bitmap);
  GlobalUnlock((HGLOBAL) bitmap_bitsH);
  GlobalFree((HGLOBAL) bitmap_bitsH);
  return((void *) bitmapH);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I s M a g i c k C o n f l i c t                                           %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  IsMagickConflict() returns true if the image format conflicts with a logical
%  drive (.e.g. X:).
%
%  The format of the IsMagickConflict method is:
%
%      MagickBooleanType IsMagickConflict(const char *magick)
%
%  A description of each parameter follows:
%
%    o magick: Specifies the image format.
%
*/
MagickExport MagickBooleanType NTIsMagickConflict(const char *magick)
{
  MagickBooleanType
    status;

  assert(magick != (char *) NULL);
  if (strlen(magick) > 1)
    return(MagickFalse);
  status=(GetLogicalDrives() & (1 << ((toupper((int) (*magick)))-'A'))) != 0 ?
    MagickTrue : MagickFalse;
  return(status);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
+   N T G e t T y pe L i s t                                                  %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  NTLoadTypeLists() loads a Windows TrueType fonts.
%
%  The format of the NTLoadTypeLists method is:
%
%      MagickBooleanType NTLoadTypeLists(SplayTreeInfo *type_list)
%
%  A description of each parameter follows:
%
%    o type_list: A linked list of fonts.
%
*/
MagickExport MagickBooleanType NTLoadTypeLists(SplayTreeInfo *type_list,
  ExceptionInfo *exception)
{
  HKEY
    reg_key = (HKEY) INVALID_HANDLE_VALUE;

  LONG
    res;


  int
    list_entries = 0;

  char
    buffer[MaxTextExtent],
    system_root[MaxTextExtent],
    font_root[MaxTextExtent];

  DWORD
    type,
    system_root_length;

  MagickBooleanType
    status;

  /*
    Try to find the right Windows*\CurrentVersion key, the SystemRoot and
    then the Fonts key
  */
  res = RegOpenKeyExA (HKEY_LOCAL_MACHINE,
    "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 0, KEY_READ, &reg_key);
  if (res == ERROR_SUCCESS) {
    system_root_length=sizeof(system_root)-1;
    res = RegQueryValueExA(reg_key,"SystemRoot",NULL, &type,
      (BYTE*) system_root, &system_root_length);
  }
  if (res != ERROR_SUCCESS) {
    res = RegOpenKeyExA (HKEY_LOCAL_MACHINE,
      "SOFTWARE\\Microsoft\\Windows\\CurrentVersion", 0, KEY_READ, &reg_key);
    if (res == ERROR_SUCCESS) {
      system_root_length=sizeof(system_root)-1;
      res = RegQueryValueExA(reg_key,"SystemRoot",NULL, &type,
        (BYTE*)system_root, &system_root_length);
    }
  }
  if (res == ERROR_SUCCESS)
    res = RegOpenKeyExA (reg_key, "Fonts",0, KEY_READ, &reg_key);
  if (res != ERROR_SUCCESS)
    return(MagickFalse);
  *font_root='\0';
  (void) CopyMagickString(buffer,system_root,MaxTextExtent);
  (void) ConcatenateMagickString(buffer,"\\fonts\\arial.ttf",MaxTextExtent);
  if (IsPathAccessible(buffer) != MagickFalse)
    {
      (void) CopyMagickString(font_root,system_root,MaxTextExtent);
      (void) ConcatenateMagickString(font_root,"\\fonts\\",MaxTextExtent);
    }
  else
    {
      (void) CopyMagickString(font_root,system_root,MaxTextExtent);
      (void) ConcatenateMagickString(font_root,"\\",MaxTextExtent);
    }

  {
    TypeInfo
      *type_info;

    DWORD
      registry_index = 0,
      type,
      value_data_size,
      value_name_length;

    char
      value_data[MaxTextExtent],
      value_name[MaxTextExtent];

    res = ERROR_SUCCESS;

    while (res != ERROR_NO_MORE_ITEMS)
      {
        char
          *family_extent,
          token[MaxTextExtent],
          *pos,
          *q;

        value_name_length = sizeof(value_name) - 1;
        value_data_size = sizeof(value_data) - 1;
        res = RegEnumValueA ( reg_key, registry_index, value_name,
          &value_name_length, 0, &type, (BYTE*)value_data, &value_data_size);
        registry_index++;
        if (res != ERROR_SUCCESS)
          continue;
        if ( (pos = strstr(value_name, " (TrueType)")) == (char*) NULL )
          continue;
        *pos='\0'; /* Remove (TrueType) from string */

        type_info=(TypeInfo *) AcquireMagickMemory(sizeof(*type_info));
        if (type_info == (TypeInfo *) NULL)
          ThrowFatalException(ResourceLimitFatalError,"MemoryAllocationFailed");
        (void) ResetMagickMemory(type_info,0,sizeof(TypeInfo));

        type_info->path=ConstantString("Windows Fonts");
        type_info->signature=MagickSignature;

        /* Name */
        (void) CopyMagickString(buffer,value_name,MaxTextExtent);
        for(pos = buffer; *pos != 0 ; pos++)
          if (*pos == ' ')
            *pos = '-';
        type_info->name=ConstantString(buffer);

        /* Fullname */
        type_info->description=ConstantString(value_name);

        /* Format */
        type_info->format=ConstantString("truetype");

        /* Glyphs */
        if (strchr(value_data,'\\') != (char *) NULL)
          (void) CopyMagickString(buffer,value_data,MaxTextExtent);
        else
          {
            (void) CopyMagickString(buffer,font_root,MaxTextExtent);
            (void) ConcatenateMagickString(buffer,value_data,MaxTextExtent);
          }

        LocaleLower(buffer);
        type_info->glyphs=ConstantString(buffer);

        type_info->stretch=NormalStretch;
        type_info->style=NormalStyle;
        type_info->weight=400;

        /* Some fonts are known to require special encodings */
        if ( (LocaleCompare(type_info->name, "Symbol") == 0 ) ||
             (LocaleCompare(type_info->name, "Wingdings") == 0 ) ||
             (LocaleCompare(type_info->name, "Wingdings-2") == 0 ) ||
             (LocaleCompare(type_info->name, "Wingdings-3") == 0 ) )
          type_info->encoding=ConstantString("AppleRoman");

        family_extent=value_name;

        for (q=value_name; *q != '\0'; )
          {
            GetMagickToken(q,(const char **) &q,token);
            if (*token == '\0')
              break;

            if (LocaleCompare(token,"Italic") == 0)
              {
                type_info->style=ItalicStyle;
              }

            else if (LocaleCompare(token,"Oblique") == 0)
              {
                type_info->style=ObliqueStyle;
              }

            else if (LocaleCompare(token,"Bold") == 0)
              {
                type_info->weight=700;
              }

            else if (LocaleCompare(token,"Thin") == 0)
              {
                type_info->weight=100;
              }

            else if ( (LocaleCompare(token,"ExtraLight") == 0) ||
                      (LocaleCompare(token,"UltraLight") == 0) )
              {
                type_info->weight=200;
              }

            else if (LocaleCompare(token,"Light") == 0)
              {
                type_info->weight=300;
              }

            else if ( (LocaleCompare(token,"Normal") == 0) ||
                      (LocaleCompare(token,"Regular") == 0) )
              {
                type_info->weight=400;
              }

            else if (LocaleCompare(token,"Medium") == 0)
              {
                type_info->weight=500;
              }

            else if ( (LocaleCompare(token,"SemiBold") == 0) ||
                      (LocaleCompare(token,"DemiBold") == 0) )
              {
                type_info->weight=600;
              }

            else if ( (LocaleCompare(token,"ExtraBold") == 0) ||
                      (LocaleCompare(token,"UltraBold") == 0) )
              {
                type_info->weight=800;
              }

            else if ( (LocaleCompare(token,"Heavy") == 0) ||
                      (LocaleCompare(token,"Black") == 0) )
              {
                type_info->weight=900;
              }

            else if (LocaleCompare(token,"Condensed") == 0)
              {
                type_info->stretch = CondensedStretch;
              }

            else if (LocaleCompare(token,"Expanded") == 0)
              {
                type_info->stretch = ExpandedStretch;
              }

            else if (LocaleCompare(token,"ExtraCondensed") == 0)
              {
                type_info->stretch = ExtraCondensedStretch;
              }

            else if (LocaleCompare(token,"ExtraExpanded") == 0)
              {
                type_info->stretch = ExtraExpandedStretch;
              }

            else if (LocaleCompare(token,"SemiCondensed") == 0)
              {
                type_info->stretch = SemiCondensedStretch;
              }

            else if (LocaleCompare(token,"SemiExpanded") == 0)
              {
                type_info->stretch = SemiExpandedStretch;
              }

            else if (LocaleCompare(token,"UltraCondensed") == 0)
              {
                type_info->stretch = UltraCondensedStretch;
              }

            else if (LocaleCompare(token,"UltraExpanded") == 0)
              {
                type_info->stretch = UltraExpandedStretch;
              }

            else
              {
                family_extent=q;
              }
          }

        (void) CopyMagickString(buffer,value_name,family_extent-value_name+1);
        StripString(buffer);
        type_info->family=ConstantString(buffer);

        list_entries++;
        status=AddValueToSplayTree(type_list,ConstantString(type_info->name),
          type_info);
        if (status == MagickFalse)
          (void) ThrowMagickException(exception,GetMagickModule(),
            ResourceLimitError,"MemoryAllocationFailed","`%s'",type_info->name);
      }
  }
  RegCloseKey ( reg_key );
  return(MagickTrue);
}

/*
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%                                                                             %
%                                                                             %
%   I m a g e T o H B i t m a p                                               %
%                                                                             %
%                                                                             %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%
%  ImageToHBITMAP() creates a Windows HBITMAP from an image.
%
%  The format of the ImageToHBITMAP method is:
%
%      HBITMAP ImageToHBITMAP(Image *image)
%
%  A description of each parameter follows:
%
%    o image: the image to convert.
%
*/
MagickExport void *ImageToHBITMAP(Image *image)
{
  BITMAP
    bitmap;

  ExceptionInfo
    *exception;

  HANDLE
    bitmap_bitsH;

  HBITMAP
    bitmapH;

  register ssize_t
    x;

  register const PixelPacket
    *p;

  register RGBQUAD
    *q;

  RGBQUAD
    *bitmap_bits;

  size_t
    length;

  ssize_t
    y;

  (void) ResetMagickMemory(&bitmap,0,sizeof(bitmap));
  bitmap.bmType=0;
  bitmap.bmWidth=(LONG) image->columns;
  bitmap.bmHeight=(LONG) image->rows;
  bitmap.bmWidthBytes=4*bitmap.bmWidth;
  bitmap.bmPlanes=1;
  bitmap.bmBitsPixel=32;
  bitmap.bmBits=NULL;
  length=bitmap.bmWidthBytes*bitmap.bmHeight;
  bitmap_bitsH=(HANDLE) GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE,length);
  if (bitmap_bitsH == NULL)
    {
      char
        *message;

      message=GetExceptionMessage(errno);
      (void) ThrowMagickException(&image->exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",message);
      message=DestroyString(message);
      return(NULL);
    }
  bitmap_bits=(RGBQUAD *) GlobalLock((HGLOBAL) bitmap_bitsH);
  q=bitmap_bits;
  if (bitmap.bmBits == NULL)
    bitmap.bmBits=bitmap_bits;
  (void) SetImageColorspace(image,sRGBColorspace);
  exception=(&image->exception);
  for (y=0; y < (ssize_t) image->rows; y++)
  {
    p=GetVirtualPixels(image,0,y,image->columns,1,exception);
    if (p == (const PixelPacket *) NULL)
      break;
    for (x=0; x < (ssize_t) image->columns; x++)
    {
      q->rgbRed=ScaleQuantumToChar(GetPixelRed(p));
      q->rgbGreen=ScaleQuantumToChar(GetPixelGreen(p));
      q->rgbBlue=ScaleQuantumToChar(GetPixelBlue(p));
      q->rgbReserved=0;
      p++;
      q++;
    }
  }
  bitmap.bmBits=bitmap_bits;
  bitmapH=CreateBitmapIndirect(&bitmap);
  if (bitmapH == NULL)
    {
      char
        *message;

      message=GetExceptionMessage(errno);
      (void) ThrowMagickException(&image->exception,GetMagickModule(),
        ResourceLimitError,"MemoryAllocationFailed","`%s'",message);
      message=DestroyString(message);
    }
  GlobalUnlock((HGLOBAL) bitmap_bitsH);
  GlobalFree((HGLOBAL) bitmap_bitsH);
  return((void *) bitmapH);
}

#endif
