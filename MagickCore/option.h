/*
  Copyright 1999-2012 ImageMagick Studio LLC, a non-profit organization
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
  MagickPixelChannelOptions,
  MagickPixelTraitOptions,
  MagickPolicyOptions,
  MagickPolicyDomainOptions,
  MagickPolicyRightsOptions,
  MagickPreviewOptions,
  MagickPrimitiveOptions,
  MagickQuantumFormatOptions,
  MagickResolutionOptions,
  MagickResourceOptions,
  MagickSparseColorOptions,
  MagickStatisticOptions,
  MagickStorageOptions,
  MagickStretchOptions,
  MagickStyleOptions,
  MagickThresholdOptions,
  MagickTypeOptions,
  MagickValidateOptions,
  MagickVirtualPixelOptions
} CommandOption;

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
    type,
    flags;

  MagickBooleanType
    stealth;
} OptionInfo;

/*
  Flags to describe classes of image processing options.
  These are used to determine how a option should be processed, and
  allow use to avoid attempting to process each option in every way posible.
*/
typedef enum
{
  UndefinedOptionFlag       = 0x0000,  /* option flag is not in use */

  ImageInfoOptionFlag       = 0x0001,  /* Setting for ImageInfo */
  DrawInfoOptionFlag        = 0x0002,  /* Setting for DrawInfo */
  QuantizeInfoOptionFlag    = 0x0004,  /* Setting for QuantizeInfo */
  GlobalOptionFlag          = 0x0008,  /* Setting for Global Option - depreciate */
  SettingOptionFlags        = 0x000F,  /* mask for all setting options */

  SimpleOperatorOptionFlag  = 0x0010,  /* Simple Image processing operator */
  ListOperatorOptionFlag    = 0x0020,  /* Multi-Image processing operator */
  SpecialOptionFlag         = 0x0040,  /* Special handled Option */
  GenesisOptionFlag         = 0x0080,  /* MagickCommandGenesis() Only Option */

  ImageRequiredFlags        = 0x0030,  /* Flags also means Images Required */

  NonMagickOptionFlag       = 0x1000,  /* Option not used by Magick Command */
  FireOptionFlag            = 0x2000,  /* Convert operation seq firing point */
  DeprecateOptionFlag       = 0x4000,  /* Deprecate option (no code) */
  ReplacedOptionFlag        = 0x8000   /* Replaced Option (still works) */

} CommandOptionFlags;


extern MagickExport char
  **GetCommandOptions(const CommandOption),
  *GetNextImageOption(const ImageInfo *),
  *RemoveImageOption(ImageInfo *,const char *);

extern MagickExport const char
  *CommandOptionToMnemonic(const CommandOption,const ssize_t),
  *GetImageOption(const ImageInfo *,const char *);

extern MagickExport MagickBooleanType
  CloneImageOptions(ImageInfo *,const ImageInfo *),
  DefineImageOption(ImageInfo *,const char *),
  DeleteImageOption(ImageInfo *,const char *),
  IsCommandOption(const char *),
  ListCommandOptions(FILE *,const CommandOption,ExceptionInfo *),
  SetImageOption(ImageInfo *,const char *,const char *);

extern MagickExport ssize_t
  GetCommandOptionFlags(const CommandOption,const MagickBooleanType,
    const char *),
  ParseChannelOption(const char *),
  ParsePixelChannelOption(const char *),
  ParseCommandOption(const CommandOption,const MagickBooleanType,const char *);

extern MagickExport void
  DestroyImageOptions(ImageInfo *),
  ResetImageOptions(const ImageInfo *),
  ResetImageOptionIterator(const ImageInfo *);

extern MagickExport const OptionInfo
  *GetCommandOptionInfo(const char *value);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
