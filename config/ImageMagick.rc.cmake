#include "winver.h"
#define __WINDOWS__
#include "..\\..\\MagickCore\\magick-baseconfig.h"
#include "..\\..\\MagickCore\\version.h"

/////////////////////////////////////////////////////////////////////////////
//
// Version
//
/////////////////////////////////////////////////////////////////////////////

VS_VERSION_INFO VERSIONINFO
 FILEVERSION @MagickLibVersionNumber@
 PRODUCTVERSION @MagickLibVersionNumber@
 FILEFLAGSMASK 0x3fL
#ifdef _DEBUG
 FILEFLAGS 0x1L
#else
 FILEFLAGS 0x0L
#endif
 FILEOS 0x40004L
 FILETYPE 0x1L
 FILESUBTYPE 0x0L
BEGIN
    BLOCK "StringFileInfo"
    BEGIN
        BLOCK "040904b0"
        BEGIN
            VALUE "ProductName", "ImageMagick\0"
            VALUE "FileDescription", "ImageMagick Studio library and utility programs\0"
            VALUE "OriginalFilename", "ImageMagick\0"
            VALUE "InternalName", "ImageMagick\0"
            VALUE "FileVersion", @MagickLibVersionText@ "\0"
            VALUE "ProductVersion", @MagickLibVersionText@ "\0"
            VALUE "CompanyName", "ImageMagick Studio\0"
            VALUE "LegalCopyright", @MagickCopyright@ "\0"
            VALUE "Comments", @MagickVersion@ "\0"
        END
    END
    BLOCK "VarFileInfo"
    BEGIN
        VALUE "Translation", 0x409, 1200
    END
END

/////////////////////////////////////////////////////////////////////////////
//
// IMAGEMAGICK
//
/////////////////////////////////////////////////////////////////////////////

// TODO generate the xml files
//COLORS.XML           IMAGEMAGICK DISCARDABLE "..\\bin\\colors.xml"
//CONFIGURE.XML        IMAGEMAGICK DISCARDABLE "..\\bin\\configure.xml"
//DELEGATES.XML        IMAGEMAGICK DISCARDABLE "..\\bin\\delegates.xml"
//ENGLISH.XML          IMAGEMAGICK DISCARDABLE "..\\bin\\english.xml"
//LOCALE.XML           IMAGEMAGICK DISCARDABLE "..\\bin\\locale.xml"
//LOG.XML              IMAGEMAGICK DISCARDABLE "..\\bin\\log.xml"
//THRESHOLDS.XML       IMAGEMAGICK DISCARDABLE "..\\bin\\thresholds.xml"
//TYPE.XML             IMAGEMAGICK DISCARDABLE "..\\bin\\type.xml"
//TYPE-GHOSTSCRIPT.XML IMAGEMAGICK DISCARDABLE "..\\bin\\type-ghostscript.xml"


/////////////////////////////////////////////////////////////////////////////
//
// Icon
//
/////////////////////////////////////////////////////////////////////////////

IDR_MAGICKICON          ICON    DISCARDABLE     "..\\..\\images\\ImageMagick.ico"
