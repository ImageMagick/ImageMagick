# ImageMagick

ImageMagick fork of original ImageMagick [repository](https://github.com/ImageMagick/ImageMagick)

## Motivation

Current ImageMagick fork was created for convenient usage MagickCore and MagicWand libraries in Xcode projects with ability to debug and modify original ImageMagick functionality using Xcode IDE.

## Instruction for integration ImageMagick libraries into user's project.

1. Add current repository as submodule to user's project repository.
2. Add ImageMagick.xcodeproj project, located in ImageMagick's root directory, into user's project or workspace as subproject.
3. Go to "Build Phases" tab for target, which use ImageMagick functionality.
4. Disclose "Link Binary With Libraries" section. Add needed libraries to link with target: libMagickCore.a or libMagickWand.a. Depending on functionality used in user's project, it may be required to link target with additional system dynamic libraries (libz, libbz2, etc).  
5. For debugging purposes uncomment string "GCC_OPTIMIZATION_LEVEL = 0" in "Debug.xccongif" configuration file.
