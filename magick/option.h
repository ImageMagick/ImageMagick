/*
  Copyright 1999-2011 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    http://www.imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore option methods.
*/
#ifndef _MAGICKCORE_OPTION_H
#define _MAGICKCORE_OPTION_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef enum
{
  MagickUndefinedOptions = -1,
  MagickAlignOptions = 0,
  MagickAlphaOptions,
  MagickBooleanOptions,
  MagickChannelOptions,
  MagickClassOptions,
  MagickClipPathOptions,
  MagickCoderOptions,
  MagickColorOptions,
  MagickColorspaceOptions,
  MagickCommandOptions,
  MagickComposeOptions,
  MagickCompressOptions,
  MagickConfigureOptions,
  MagickDataTypeOptions,
  MagickDebugOptions,
  MagickDecorateOptions,
  MagickDelegateOptions,
  MagickDirectionOptions,
  MagickDisposeOptions,
  MagickDistortOptions,
  MagickDitherOptions,
  MagickEndianOptions,
  MagickEvaluateOptions,
  MagickFillRuleOptions,
  MagickFilterOptions,
  MagickFontOptions,
  MagickFontsOptions,
  MagickFormatOptions,
  MagickFunctionOptions,
  MagickGravityOptions,
  MagickImageListOptions,
  MagickIntentOptions,
  MagickInterlaceOptions,
  MagickInterpolateOptions,
  MagickKernelOptions,
  MagickLayerOptions,
  MagickLineCapOptions,
  MagickLineJoinOptions,
  MagickListOptions,
  MagickLocaleOptions,
  MagickLogEventOptions,
  MagickLogOptions,
  MagickMagicOptions,
  MagickMethodOptions,
  MagickMetricOptions,
  MagickMimeOptions,
  MagickModeOptions,
  MagickModuleOptions,
  MagickMorphologyOptions,
  MagickNoiseOptions,
  MagickOrientationOptions,
  MagickPolicyOptions,
  MagickPolicyDomainOptions,
  MagickPolicyRightsOptions,
  MagickPreviewOptions,
  MagickPrimitiveOptions,
  MagickQuantumFormatOptions,
  MagickResolutionOptions,
  MagickResourceOptions,
  MagickSparseColorOptions,
  MagickStorageOptions,
  MagickStretchOptions,
  MagickStyleOptions,
  MagickThresholdOptions,
  MagickTypeOptions,
  MagickValidateOptions,
  MagickVirtualPixelOptions
} MagickOption;

typedef enum
{
  UndefinedValidate,
  NoValidate = 0x00000,
  CompareValidate = 0x00001,
  CompositeValidate = 0x00002,
  ConvertValidate = 0x00004,
  FormatsInMemoryValidate = 0x00008,
  FormatsOnDiskValidate = 0x00010,
  IdentifyValidate = 0x00020,
  ImportExportValidate = 0x00040,
  MontageValidate = 0x00080,
  StreamValidate = 0x00100,
  AllValidate = 0x7fffffff
} ValidateType;

typedef struct _OptionInfo
{
  const char
    *mnemonic;

  ssize_t
    type;

  MagickBooleanType
    stealth;
} OptionInfo;

extern MagickExport char
  **GetMagickOptions(const MagickOption),
  *GetNextImageOption(const ImageInfo *),
  *RemoveImageOption(ImageInfo *,const char *);

extern MagickExport const char
  *GetImageOption(const ImageInfo *,const char *),
  *MagickOptionToMnemonic(const MagickOption,const ssize_t);

extern MagickExport MagickBooleanType
  CloneImageOptions(ImageInfo *,const ImageInfo *),
  DefineImageOption(ImageInfo *,const char *),
  DeleteImageOption(ImageInfo *,const char *),
  IsMagickOption(const char *),
  ListMagickOptions(FILE *,const MagickOption,ExceptionInfo *),
  SetImageOption(ImageInfo *,const char *,const char *);

extern MagickExport ssize_t
  ParseChannelOption(const char *),
  ParseMagickOption(const MagickOption,const MagickBooleanType,const char *);

extern MagickExport void
  DestroyImageOptions(ImageInfo *),
  ResetImageOptions(const ImageInfo *),
  ResetImageOptionIterator(const ImageInfo *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
