#  Copyright 1999-2018 ImageMagick Studio LLC, a non-profit organization
#  dedicated to making software imaging solutions freely available.
#
#  You may not use this file except in compliance with the License.  You may
#  obtain a copy of the License at
#
#    http://www.imagemagick.org/script/license.php
#
#  Unless required by applicable law or agreed to in writing, software
#  distributed under the License is distributed on an "AS IS" BASIS,
#  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  See the License for the specific language governing permissions and
#  limitations under the License.
#
#  Makefile for building ImageMagick filter modules.

# Where filter modules get installed
filtersdir = $(FILTER_PATH)

MAGICK_FILTER_CPPFLAGS = $(AM_CPPFLAGS)

MAGICKCORE_FILTER_SRCS = \
	filters/analyze.c

if WITH_MODULES
filters_LTLIBRARIES = \
	filters/analyze.la
else
filters_LTLIBRARIES =
endif # WITH_MODULES
filters_CPPFLAGS = $(MAGICK_FILTER_CPPFLAGS)

# analyze filter module
filters_analyze_la_SOURCES      = filters/analyze.c
filters_analyze_la_CPPFLAGS     = $(MAGICK_FILTER_CPPFLAGS)
filters_analyze_la_LDFLAGS      = $(MODULECOMMONFLAGS)
filters_analyze_la_LIBADD       = $(MAGICKCORE_LIBS) $(MATH_LIBS)
