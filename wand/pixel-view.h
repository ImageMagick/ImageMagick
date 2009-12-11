/*
  Copyright 1999-2010 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITTransferNS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickWand pixel view methods.
*/
#ifndef _MAGICKWAND_PIXEL_VIEW_H
#define _MAGICKWAND_PIXEL_VIEW_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _PixelView
  PixelView;

typedef MagickBooleanType
  (*DuplexTransferPixelViewMethod)(const PixelView *,const PixelView *,
    PixelView *,void *),
  (*GetPixelViewMethod)(const PixelView *,void *),
  (*SetPixelViewMethod)(PixelView *,void *),
  (*TransferPixelViewMethod)(const PixelView *,PixelView *,void *),
  (*UpdatePixelViewMethod)(PixelView *,void *);

extern WandExport char
  *GetPixelViewException(const PixelView *,ExceptionType *);

extern WandExport long
  GetPixelViewX(const PixelView *),
  GetPixelViewY(const PixelView *);

extern WandExport MagickBooleanType
  DuplexTransferPixelViewIterator(PixelView *,PixelView *,PixelView *,
    DuplexTransferPixelViewMethod,void *),
  GetPixelViewIterator(PixelView *,GetPixelViewMethod,void *),
  IsPixelView(const PixelView *),
  SetPixelViewIterator(PixelView *,SetPixelViewMethod,void *),
  TransferPixelViewIterator(PixelView *,PixelView *,TransferPixelViewMethod,
    void *),
  UpdatePixelViewIterator(PixelView *,UpdatePixelViewMethod,void *);

extern WandExport MagickWand
  *GetPixelViewWand(const PixelView *);

extern WandExport PixelView
  *ClonePixelView(const PixelView *),
  *DestroyPixelView(PixelView *),
  *NewPixelView(MagickWand *),
  *NewPixelViewRegion(MagickWand *,const long,const long,
    const unsigned long,const unsigned long);

extern WandExport PixelWand
  **GetPixelViewPixels(const PixelView *);

extern WandExport unsigned long
  GetPixelViewHeight(const PixelView *),
  GetPixelViewWidth(const PixelView *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
