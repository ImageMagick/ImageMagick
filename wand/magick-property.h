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

  MagickWand property, options, and profile  methods.
*/

#ifndef _MAGICKWAND_MAGICK_PROPERTY_H
#define _MAGICKWAND_MAGICK_PROPERTY_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

extern WandExport char
  *MagickGetFilename(const MagickWand *),
  *MagickGetFormat(MagickWand *),
  *MagickGetFont(MagickWand *),
  *MagickGetHomeURL(void),
  *MagickGetImageArtifact(MagickWand *,const char *),
  **MagickGetImageArtifacts(MagickWand *,const char *,unsigned long *),
  **MagickGetImageProfiles(MagickWand *,const char *,unsigned long *),
  *MagickGetImageProperty(MagickWand *,const char *),
  **MagickGetImageProperties(MagickWand *,const char *,unsigned long *),
  *MagickGetOption(MagickWand *,const char *),
  **MagickGetOptions(MagickWand *,const char *,unsigned long *),
  *MagickQueryConfigureOption(const char *),
  **MagickQueryConfigureOptions(const char *,unsigned long *),
  **MagickQueryFonts(const char *,unsigned long *),
  **MagickQueryFormats(const char *,unsigned long *);

extern WandExport ColorspaceType
  MagickGetColorspace(MagickWand *);

extern WandExport CompressionType
  MagickGetCompression(MagickWand *);

extern WandExport const char
  *MagickGetCopyright(void),
  *MagickGetPackageName(void),
  *MagickGetQuantumDepth(unsigned long *),
  *MagickGetQuantumRange(unsigned long *),
  *MagickGetReleaseDate(void),
  *MagickGetVersion(unsigned long *);

extern WandExport double
  MagickGetPointsize(MagickWand *),
  *MagickGetSamplingFactors(MagickWand *,unsigned long *),
  *MagickQueryFontMetrics(MagickWand *,const DrawingWand *,const char *),
  *MagickQueryMultilineFontMetrics(MagickWand *,const DrawingWand *,
    const char *);

extern WandExport GravityType
  MagickGetGravity(MagickWand *);

extern WandExport ImageType
  MagickGetType(MagickWand *);

extern WandExport InterlaceType
  MagickGetInterlaceScheme(MagickWand *);

extern WandExport InterpolatePixelMethod
  MagickGetInterpolateMethod(MagickWand *);

extern WandExport OrientationType
  MagickGetOrientation(MagickWand *);

extern WandExport MagickBooleanType
  MagickDeleteImageArtifact(MagickWand *,const char *),
  MagickDeleteImageProperty(MagickWand *,const char *),
  MagickDeleteOption(MagickWand *,const char *),
  MagickGetAntialias(const MagickWand *),
  MagickGetPage(const MagickWand *,unsigned long *,unsigned long *,long *,
    long *),
  MagickGetSize(const MagickWand *,unsigned long *,unsigned long *),
  MagickGetSizeOffset(const MagickWand *,long *),
  MagickProfileImage(MagickWand *,const char *,const void *,const size_t),
  MagickSetAntialias(MagickWand *,const MagickBooleanType),
  MagickSetBackgroundColor(MagickWand *,const PixelWand *),
  MagickSetColorspace(MagickWand *,const ColorspaceType),
  MagickSetCompression(MagickWand *,const CompressionType),
  MagickSetCompressionQuality(MagickWand *,const unsigned long),
  MagickSetDepth(MagickWand *,const unsigned long),
  MagickSetExtract(MagickWand *,const char *),
  MagickSetFilename(MagickWand *,const char *),
  MagickSetFormat(MagickWand *,const char *),
  MagickSetFont(MagickWand *,const char *),
  MagickSetGravity(MagickWand *,const GravityType),
  MagickSetImageArtifact(MagickWand *,const char *,const char *),
  MagickSetImageProfile(MagickWand *,const char *,const void *,const size_t),
  MagickSetImageProperty(MagickWand *,const char *,const char *),
  MagickSetInterlaceScheme(MagickWand *,const InterlaceType),
  MagickSetInterpolateMethod(MagickWand *,const InterpolatePixelMethod),
  MagickSetOption(MagickWand *,const char *,const char *),
  MagickSetOrientation(MagickWand *,const OrientationType),
  MagickSetPage(MagickWand *,const unsigned long,const unsigned long,
    const long,const long),
  MagickSetPassphrase(MagickWand *,const char *),
  MagickSetPointsize(MagickWand *,const double),
  MagickSetResolution(MagickWand *,const double,const double),
  MagickSetResourceLimit(const ResourceType type,const MagickSizeType limit),
  MagickSetSamplingFactors(MagickWand *,const unsigned long,const double *),
  MagickSetSize(MagickWand *,const unsigned long,const unsigned long),
  MagickSetSizeOffset(MagickWand *,const unsigned long,const unsigned long,
    const long),
  MagickSetType(MagickWand *,const ImageType);

extern WandExport MagickProgressMonitor
  MagickSetProgressMonitor(MagickWand *,const MagickProgressMonitor,void *);

extern WandExport MagickSizeType
  MagickGetResource(const ResourceType),
  MagickGetResourceLimit(const ResourceType);

extern WandExport PixelWand
  *MagickGetBackgroundColor(MagickWand *);

extern WandExport OrientationType
  MagickGetOrientationType(MagickWand *);

extern WandExport unsigned char
  *MagickGetImageProfile(MagickWand *,const char *,size_t *),
  *MagickRemoveImageProfile(MagickWand *,const char *,size_t *);

extern WandExport unsigned long
  MagickGetCompressionQuality(MagickWand *);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
