/*
  Copyright 1999-2020 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at
  
    https://imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickWand mogrify command-line method.
*/
#ifndef MAGICKWAND_MOGRIFY_H
#define MAGICKWAND_MOGRIFY_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef MagickBooleanType
  (*MagickCommand)(ImageInfo *,int,char **,char **,ExceptionInfo *);

extern WandExport MagickBooleanType
  MagickCommandGenesis(ImageInfo *,MagickCommand,int,char **,char **,
    ExceptionInfo *),
  MogrifyImage(ImageInfo *,const int,const char **,Image **,ExceptionInfo *),
  MogrifyImageCommand(ImageInfo *,int,char **,char **,ExceptionInfo *),
  MogrifyImageInfo(ImageInfo *,const int,const char **,ExceptionInfo *),
  MogrifyImageList(ImageInfo *,const int,const char **,Image **,
    ExceptionInfo *),
  MogrifyImages(ImageInfo *,const MagickBooleanType,const int,const char **,
    Image **,ExceptionInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
