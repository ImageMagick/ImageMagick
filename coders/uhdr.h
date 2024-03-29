/*
  Copyright @ 2024 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.

  You may not use this file except in compliance with the License.  You may
  obtain a copy of the License at

    https://imagemagick.org/script/license.php

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

MagickBooleanType IsUHDR(const ImageInfo *image_info, ExceptionInfo *exception);

Image *ReadUHDRImage(const ImageInfo *image_info, ExceptionInfo *exception);

MagickBooleanType HasResourcesForUHdrEncode(const Image *images);

MagickBooleanType WriteUHDRImage(const ImageInfo *image_info, Image *image,
                                 ExceptionInfo *exception);

#if defined(__cplusplus) || defined(c_plusplus)
}
#endif