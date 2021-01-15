#  Copyright 1999-2021 ImageMagick Studio LLC, a non-profit organization
#  dedicated to making software imaging solutions freely available.
#
#  You may not use this file except in compliance with the License.  You may
#  obtain a copy of the License at
#
#    https://imagemagick.org/script/license.php
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  ImageMagick versioning.
#
m4_define([magick_name], [ImageMagick])
m4_define([magick_major_version], [7])
m4_define([magick_minor_version], [0])
m4_define([magick_micro_version], [10])
m4_define([magick_patchlevel_version], [58])
m4_define([magick_base_version],
          [magick_major_version.magick_minor_version.magick_micro_version])
m4_define([magick_version],
          [magick_base_version-magick_patchlevel_version])
m4_define([magick_bugreport],
          [https://github.com/ImageMagick/ImageMagick/issues])
m4_define([magick_url], [https://imagemagick.org])
m4_define([magick_lib_version],[0x70A])
m4_define([magick_lib_version_number],
          [magick_major_version,magick_minor_version,magick_micro_version,magick_patchlevel_version])
m4_define([magick_git_revision],
          esyscmd([
            c=$(git log --full-history --format=tformat:. HEAD | wc -l)
            h=$(git rev-parse --short HEAD)
            d=$(date +%Y%m%d)
            printf %s "$c:$h:$d"
          ]))
m4_define([magick_tarname],[ImageMagick])
