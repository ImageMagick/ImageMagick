/*
  Copyright 1999-2019 ImageMagick Studio LLC, a non-profit organization
  dedicated to making software imaging solutions freely available.
  
  You may not use this file except in compliance with the License.
  obtain a copy of the License at
  
    https://imagemagick.org/script/license.php
  
  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.
*/
#ifndef MAGICK_CODERS_PRIVATE_H
#define MAGICK_CODERS_PRIVATE_H

#define MagickCoderHeader(coder,offset,magic)  { coder, offset, \
  (const unsigned char *) (magic), sizeof(magic)-1 },

#define MagickCoderAlias(coder,alias)  { alias, coder },

#define MagickCoderExports(coder) \
extern ModuleExport size_t \
  Register ## coder ## Image(void); \
extern ModuleExport void \
  Unregister ## coder ## Image(void);

#endif