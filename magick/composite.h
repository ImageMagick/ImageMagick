/*
  Copyright 1999-2010 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore image composite methods.
*/
#ifndef _MAGICKCORE_COMPOSITE_H
#define _MAGICKCORE_COMPOSITE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef enum
{
  UndefinedCompositeOp,
  NoCompositeOp,
  AddCompositeOp,
  AtopCompositeOp,
  BlendCompositeOp,
  BumpmapCompositeOp,
  ChangeMaskCompositeOp,
  ClearCompositeOp,
  ColorBurnCompositeOp,
  ColorDodgeCompositeOp,
  ColorizeCompositeOp,
  CopyBlackCompositeOp,
  CopyBlueCompositeOp,
  CopyCompositeOp,
  CopyCyanCompositeOp,
  CopyGreenCompositeOp,
  CopyMagentaCompositeOp,
  CopyOpacityCompositeOp,
  CopyRedCompositeOp,
  CopyYellowCompositeOp,
  DarkenCompositeOp,
  DstAtopCompositeOp,
  DstCompositeOp,
  DstInCompositeOp,
  DstOutCompositeOp,
  DstOverCompositeOp,
  DifferenceCompositeOp,
  DisplaceCompositeOp,
  DissolveCompositeOp,
  ExclusionCompositeOp,
  HardLightCompositeOp,
  HueCompositeOp,
  InCompositeOp,
  LightenCompositeOp,
  LinearLightCompositeOp,
  LuminizeCompositeOp,
  MinusCompositeOp,
  ModulateCompositeOp,
  MultiplyCompositeOp,
  OutCompositeOp,
  OverCompositeOp,
  OverlayCompositeOp,
  PlusCompositeOp,
  ReplaceCompositeOp,
  SaturateCompositeOp,
  ScreenCompositeOp,
  SoftLightCompositeOp,
  SrcAtopCompositeOp,
  SrcCompositeOp,
  SrcInCompositeOp,
  SrcOutCompositeOp,
  SrcOverCompositeOp,
  SubtractCompositeOp,
  ThresholdCompositeOp,
  XorCompositeOp,
  DivideCompositeOp,
  DistortCompositeOp,
  BlurCompositeOp,
  PegtopLightCompositeOp,
  VividLightCompositeOp,
  PinLightCompositeOp,
  LinearDodgeCompositeOp,
  LinearBurnCompositeOp,
  MathematicsCompositeOp
} CompositeOperator;

extern MagickExport MagickBooleanType
  CompositeImage(Image *,const CompositeOperator,const Image *,const long,
    const long),
  CompositeImageChannel(Image *,const ChannelType,const CompositeOperator,
    const Image *,const long,const long),
  TextureImage(Image *,const Image *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
