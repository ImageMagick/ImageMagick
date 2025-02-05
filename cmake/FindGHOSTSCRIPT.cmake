
#--------------------------------------------------------------------------------
# Copyright (c) 2012-2013, Lars Baehren <lbaehren@gmail.com>
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
#  * Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
# FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
# SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
# OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#--------------------------------------------------------------------------------

# - Check for the presence of GHOSTSCRIPT
#
# The following variables are set when GHOSTSCRIPT is found:
#  GHOSTSCRIPT_FOUND      = Set to true, if all components of GHOSTSCRIPT have been found.
#  GHOSTSCRIPT_INCLUDES   = Include path for the header files of GHOSTSCRIPT
#  GHOSTSCRIPT_LIBRARIES  = Link these to use GHOSTSCRIPT
#  GHOSTSCRIPT_LFLAGS     = Linker flags (optional)

if (NOT GHOSTSCRIPT_FOUND)

  if (NOT GHOSTSCRIPT_ROOT_DIR)
    set (GHOSTSCRIPT_ROOT_DIR ${CMAKE_INSTALL_PREFIX})
  endif (NOT GHOSTSCRIPT_ROOT_DIR)

  ##_____________________________________________________________________________
  ## Check for the header files

  find_path (GHOSTSCRIPT_INCLUDES
    NAMES ghostscript/gdevdsp.h ghostscript/iapi.h
    HINTS ${GHOSTSCRIPT_ROOT_DIR} ${CMAKE_INSTALL_PREFIX}
    PATH_SUFFIXES include
    )

  ##_____________________________________________________________________________
  ## Check for the library

  find_library (GHOSTSCRIPT_LIBRARIES gs
    HINTS ${GHOSTSCRIPT_ROOT_DIR} ${CMAKE_INSTALL_PREFIX}
    PATH_SUFFIXES lib
    )

  ##_____________________________________________________________________________
  ## Check for the executable

  find_program (GS_EXECUTABLE gs
    HINTS ${GHOSTSCRIPT_ROOT_DIR} ${CMAKE_INSTALL_PREFIX}
    PATH_SUFFIXES bin
    )

  find_program (EPS2EPS_EXECUTABLE eps2eps
    HINTS ${GHOSTSCRIPT_ROOT_DIR} ${CMAKE_INSTALL_PREFIX}
    PATH_SUFFIXES bin
    )
  
  find_program (DVIPDF_EXECUTABLE dvipdf
    HINTS ${GHOSTSCRIPT_ROOT_DIR} ${CMAKE_INSTALL_PREFIX}
    PATH_SUFFIXES bin
    )
  
  find_program (PS2PS2_EXECUTABLE ps2ps2
    HINTS ${GHOSTSCRIPT_ROOT_DIR} ${CMAKE_INSTALL_PREFIX}
    PATH_SUFFIXES bin
    )

  ##_____________________________________________________________________________
  ## Actions taken when all components have been found

  find_package_handle_standard_args (GHOSTSCRIPT DEFAULT_MSG GHOSTSCRIPT_LIBRARIES GHOSTSCRIPT_INCLUDES GS_EXECUTABLE)

  if (GHOSTSCRIPT_FOUND)
    if (NOT GHOSTSCRIPT_FIND_QUIETLY)
      message (STATUS "Found components for GHOSTSCRIPT")
      message (STATUS "GHOSTSCRIPT_ROOT_DIR  = ${GHOSTSCRIPT_ROOT_DIR}")
      message (STATUS "GHOSTSCRIPT_INCLUDES  = ${GHOSTSCRIPT_INCLUDES}")
      message (STATUS "GHOSTSCRIPT_LIBRARIES = ${GHOSTSCRIPT_LIBRARIES}")
    endif (NOT GHOSTSCRIPT_FIND_QUIETLY)
  else (GHOSTSCRIPT_FOUND)
    if (GHOSTSCRIPT_FIND_REQUIRED)
      message (FATAL_ERROR "Could not find GHOSTSCRIPT!")
    endif (GHOSTSCRIPT_FIND_REQUIRED)
  endif (GHOSTSCRIPT_FOUND)

  ##_____________________________________________________________________________
  ## Mark advanced variables

  mark_as_advanced (
    GHOSTSCRIPT_ROOT_DIR
    GHOSTSCRIPT_INCLUDES
    GHOSTSCRIPT_LIBRARIES
    GS_EXECUTABLE
    )

endif (NOT GHOSTSCRIPT_FOUND)
