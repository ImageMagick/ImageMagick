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

  ImageMagick pixel wand API.
*/
#ifndef _MAGICK_CLI_PRIVATE_H
#define _MAGICK_CLI_PRIVATE_H

#if defined(__cplusplus) || defined(c_plusplus)
extern "C" {
#endif

/* Define a generic stack linked list, for pushing and popping
   user defined ImageInfo settings, and Image lists.
   See '(' ')' and '-clone' CLI options.
*/
typedef struct _Stack
{
  struct _Stack  *next;
  void           *data;
} Stack;

/* Note this defines an extension to the normal MagickWand
   Which adds extra elements specific to the Shell API interface
   while still allowing the Wand to be passed to MagickWand API
   for specific operations.
*/
struct _MagickCLI       /* CLI interface version of MagickWand */
{
  struct _MagickWand    /* this must be the first structure */
     wand;

  QuantizeInfo
    *quantize_info;     /* for CLI API usage, not used by MagickWand API */

  DrawInfo
    *draw_info;         /* for CLI API usage, not used by MagickWand API */

  Stack
    *image_list_stack,  /* Stacks of Image Lists and Image Info settings */
    *image_info_stack;

  size_t
    signature;
};


#if defined(__cplusplus) || defined(c_plusplus)
}
#endif

#endif
