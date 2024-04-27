from conan import ConanFile
from conan.tools.files import save
from conan.tools import CppInfo
import os


required_conan_version = ">=2.0.0"


class ImageMagickDelegates(ConanFile):
    settings = 'os', 'compiler', 'build_type', 'arch'

    options = {
      'libtype': [ 'shared', 'static' ],
      'fonts': [ True, False ],
      'jpeg': [ True, False ],
      'png': [ True, False ],
      'tiff': [ True, False ],
      'webp': [ True, False ],
      'jpeg2000': [ True, False ],
      'raw': [ True, False ],
      'openmedia': [ True, False ],
      'brotli': [ True, False ],
      'h265': [ True, False ],
      'exr': [ True, False ],
      'fftw': [ True, False ],
      'heif': [ True, False ],
      'jbig': [ True, False ],
      'color': [ True, False ],
      'xml': [ True, False ],
      'gzip': [ True, False ],
      'zip': [ True, False ],
      'bzip2': [ True, False ],
      'zstd': [ True, False ],
      'xz': [ True, False ],
      'lzma': [ True, False ],
      'cairo': [ True, False ],
      'jemalloc': [ True, False ],
      'simd': [ True, False ],
      'opencl': [ True, False ],
      'openmp': [ True, False ],
      'display': [ True, False ]
    }

    default_options = {
      'libtype': 'static',
      'fonts': True,
      'jpeg': True,
      'png': True,
      'tiff': True,
      'webp': True,
      'jpeg2000': True,
      'raw': True,
      'openmedia': True,
      'brotli': True,
      'h265': True,
      'exr': True,
      'fftw': True,
      'heif': True,
      'jbig': True,
      'color': True,
      'xml': True,
      'gzip':True,
      'zip': True,
      'bzip2': True,
      'zstd': True,
      'xz': True,
      'lzma': True,
      'cairo': True,
      'jemalloc': True,
      'simd': True,
      'opencl': False,
      'openmp': True,
      'display': True
    }

    generators = [ 'CMakeDeps', 'CMakeToolchain', 'VCVars', 'VirtualRunEnv' ]

    # Used only for testing the pkg-config build in Github Actions
    if ('CONAN_GENERATOR' in os.environ):
      generators = os.environ['CONAN_GENERATOR']

    def requirements(self):
      if self.options.libtype == 'static':
        self.default_options = { '*:shared': False }
      if self.options.libtype == 'shared':
        self.default_options = { '*:shared': True }

      # Fonts are not available on WASM targets
      if self.options.fonts and self.settings.arch != 'wasm':
        [self.requires(x, force=True) for x in
          ('libffi/3.4.4', 'fontconfig/2.14.2', 'freetype/2.13.2', 'fribidi/1.0.12', 'glib/2.78.1', 'harfbuzz/8.3.0')]

      # LZMA for WASM is blocked by https://github.com/conan-io/conan-center-index/issues/20602
      if self.options.lzma and self.settings.arch != 'wasm':
        self.requires('lzma_sdk/9.20', force=True)

      if self.options.bzip2:
        self.requires('bzip2/1.0.8', force=True)

      if self.options.zstd:
        self.requires('zstd/1.5.5', force=True)

      if self.options.zip:
        self.requires('libzip/1.9.2', force=True)

      if self.options.brotli:
        self.requires('brotli/1.1.0', force=True)

      if self.options.xz:
        self.requires('xz_utils/5.4.5', force=True)

      if self.options.gzip:
        self.requires('zlib/1.2.13', force=True)

      if self.options.fftw:
        self.requires('fftw/3.3.10', force=True)

      if self.options.color:
        self.requires('lcms/2.14', force=True)

      if self.options.xml:
        self.requires('libxml2/2.10.4', force=True)

      if self.options.openmedia:
        self.requires('libaom-av1/3.6.0', force=True)

      if self.options.h265:
        self.requires('libde265/1.0.12', force=True)

      if self.options.heif:
        self.requires('libheif/1.13.0', force=True)

      if self.options.jbig:
        self.requires('jbig/20160605', force=True)

      if self.options.exr:
        self.requires('openexr/3.1.5', force=True)

      if self.options.png:
        self.requires('libpng/1.6.42', force=True)

      if self.options.webp:
        self.requires('libwebp/1.3.2', force=True)

      if self.options.jpeg2000 or self.options.tiff or self.options.raw:
        self.requires('libjpeg-turbo/3.0.2', force=True)

      if self.options.jpeg2000:
        self.requires('jasper/4.2.0', force=True)

      if self.options.tiff:
        self.requires('libtiff/4.6.0', force=True)

      if self.options.raw:
        self.requires('libraw/0.21.2', force=True)

      if self.options.jpeg:
        self.requires('openjpeg/2.5.0', force=True)

      if self.options.cairo and self.settings.arch != 'wasm':
        self.requires('cairo/1.17.8', force=True)
        self.requires('expat/2.6.0', force=True)

      if self.options.simd and self.settings.arch != 'wasm':
        self.requires('highway/1.0.3', force=True)

      if self.options.openmp and self.settings.arch != 'wasm' and self.settings.os != 'Windows':
        self.requires('llvm-openmp/17.0.6', force=True)

      if self.options.display and self.settings.arch != 'wasm':
        self.requires('pixman/0.42.2', force=True)

      # Although supported in theory, using jemalloc on Windows is very difficult especially
      # with a generic build that supports options and shared/static builds
      if self.options.jemalloc and self.settings.arch != 'wasm' and self.settings.os != 'Windows':
        self.requires('jemalloc/5.3.0', force=True)

      if self.options.opencl:
        self.requires('opencl-headers/2023.12.14', force=True)

    def configure(self):
      if self.settings.arch != 'wasm' and self.options.fonts:
        self.options['glib'].shared = False
        self.options['glib'].fPIC = True

      if self.options.jpeg2000:
        self.options['jasper'].with_libjpeg = 'libjpeg-turbo'

      if self.options.tiff:
        self.options['libtiff'].jpeg = 'libjpeg-turbo'

      if self.options.raw:
        self.options['libraw'].with_jpeg = 'libjpeg-turbo'

      if self.options.cairo and self.settings.arch != 'wasm':
        self.options['cairo'].with_png = self.options.png
        self.options['cairo'].with_glib = self.settings.arch != 'wasm' and self.options.fonts
        # There is no portable way to include xlib
        self.options['cairo'].with_xlib = False
        self.options['cairo'].with_xlib_xrender = False
        self.options['cairo'].with_xcb = False
        self.options['cairo'].with_xcb = False
        self.options['cairo'].with_zlib = self.options.gzip
        self.options['cairo'].with_freetype = self.settings.arch != 'wasm' and self.options.fonts
        self.options['cairo'].with_fontconfig = self.settings.arch != 'wasm' and self.options.fonts

      # While Emscripten supports SIMD, Node.js does not and cannot run the resulting WASM bundle
      # The performance gain is not very significant and it has a huge compatibility issue
      if self.options.webp and (self.settings.arch == 'wasm' or not self.options.simd):
        self.options['libwebp'].with_simd = False
      
      # When building with emscripten, the main exe is called zstd.js and all symlinks are broken
      if self.settings.arch == 'wasm' and self.options.zstd:
        self.options['zstd'].build_programs = False
