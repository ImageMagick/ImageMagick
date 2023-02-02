/*
  Copyright @ 2000 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at
  
    https://imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  MagickCore option methods.
*/
#ifndef MAGICKCORE_OPTION_H
#define MAGICKCORE_OPTION_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

typedef enum
{
  MagickUndefinedOptions = -1,
  MagickAlignOptions = 0,
  MagickAlphaChannelOptions,
  MagickBooleanOptions,
  MagickCacheOptions,
  MagickChannelOptions,
  MagickClassOptions,
  MagickClipPathOptions,
  MagickCoderOptions,
  MagickColorOptions,
  MagickColorspaceOptions,
  MagickCommandOptions,
  MagickComplexOptions,
  MagickComplianceOptions,
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
  MagickGradientOptions,
  MagickGravityOptions,
  MagickIntensityOptions,
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
  MagickPixelIntensityOptions,
  MagickPixelMaskOptions,
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
  MagickVirtualPixelOptions,
  MagickWeightOptions,
  MagickAutoThresholdOptions,
  MagickToolOptions,
  MagickCLIOptions,
  MagickIlluminantOptions,
  MagickWordBreakOptions,
  MagickPagesizeOptions
} CommandOption;

typedef enum
{
  UndefinedValidate,
  NoValidate = 0x00000,
  ColorspaceValidate = 0x00001,
  CompareValidate = 0x00002,
  CompositeValidate = 0x00004,
  ConvertValidate = 0x00008,
  FormatsDiskValidate = 0x00010,
  FormatsMapValidate = 0x00020,
  FormatsMemoryValidate = 0x00040,
  IdentifyValidate = 0x00080,
  ImportExportValidate = 0x00100,
  MontageValidate = 0x00200,
  StreamValidate = 0x00400,
  MagickValidate = 0x00800,
  AllValidate = 0x7fffffff
} ValidateType;

/*
  Flags to describe classes of image processing options.
  These are used to determine how a option should be processed, and
  avoid attempting to process all options in every way possible.
*/
typedef enum
{
  UndefinedOptionFlag       = 0x0000,  /* option flag is not in use */

  ImageInfoOptionFlag       = 0x0001,  /* Setting stored in ImageInfo */
  DrawInfoOptionFlag        = 0x0002,  /* Setting stored in DrawInfo */
  QuantizeInfoOptionFlag    = 0x0004,  /* Setting stored in QuantizeInfo */
  GlobalOptionFlag          = 0x0008,  /* Global Setting or Control */
  SettingOptionFlags        = 0x000F,  /* mask any setting option */

  NoImageOperatorFlag       = 0x0010,  /* Images not required operator */
  SimpleOperatorFlag        = 0x0020,  /* Simple Image processing operator */
  ListOperatorFlag          = 0x0040,  /* Multi-Image processing operator */
  GenesisOptionFlag         = 0x0080,  /* MagickCommandGenesis() Only Option */

  SpecialOptionFlag         = 0x0100,  /* Operator with Special Requirements */
                                       /* EG: for specific CLI commands */

  AlwaysInterpretArgsFlag   = 0x0400,  /* Always Interpret escapes in Args */
                                       /* CF: "convert" compatibility mode */
  NeverInterpretArgsFlag    = 0x0800,  /* Never Interpret escapes in Args */
                                       /* EG: filename, or delayed escapes */

  NonMagickOptionFlag       = 0x1000,  /* Option not used by Magick Command */
  FireOptionFlag            = 0x2000,  /* Convert operation seq firing point */
  DeprecateOptionFlag       = 0x4000,  /* Deprecate option (no code) */
  ReplacedOptionFlag        = 0x8800   /* Replaced Option (but still works) */

} CommandOptionFlags;

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
  IsOptionMember(const char *,const char *),
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
