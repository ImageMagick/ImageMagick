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

  MagickWand wand view methods.
*/
#ifndef _MAGICKWAND_WAND_VIEW_H
#define _MAGICKWAND_WAND_VIEW_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef struct _WandView
  WandView;

typedef MagickBooleanType
  (*DuplexTransferWandViewMethod)(const WandView *,const WandView *,WandView *,
    void *),
  (*GetWandViewMethod)(const WandView *,void *),
  (*SetWandViewMethod)(WandView *,void *),
  (*TransferWandViewMethod)(const WandView *,WandView *,void *),
  (*UpdateWandViewMethod)(WandView *,void *);

extern WandExport char
  *GetWandViewException(const WandView *,ExceptionType *);

extern WandExport MagickBooleanType
  DuplexTransferWandViewIterator(WandView *,WandView *,WandView *,
    DuplexTransferWandViewMethod,void *),
  GetWandViewIterator(WandView *,GetWandViewMethod,void *),
  IsWandView(const WandView *),
  SetWandViewIterator(WandView *,SetWandViewMethod,void *),
  TransferWandViewIterator(WandView *,WandView *,TransferWandViewMethod,void *),
  UpdateWandViewIterator(WandView *,UpdateWandViewMethod,void *);

extern WandExport MagickWand
  *GetWandViewWand(const WandView *);

extern WandExport WandView
  *CloneWandView(const WandView *),
  *DestroyWandView(WandView *),
  *NewWandView(MagickWand *),
  *NewWandViewRegion(MagickWand *,const ssize_t,const ssize_t,const size_t,
    const size_t);

extern WandExport PixelWand
  **GetWandViewPixels(const WandView *);

extern WandExport size_t
  GetWandViewHeight(const WandView *),
  GetWandViewWidth(const WandView *);

extern WandExport ssize_t
  GetWandViewX(const WandView *),
  GetWandViewY(const WandView *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
