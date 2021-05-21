# Adapted from upstream:
# https://github.com/msys2/MINGW-packages/tree/master/mingw-w64-imagemagick

_realname=imagemagick
pkgbase="mingw-w64-${_realname}"
pkgname="${MINGW_PACKAGE_PREFIX}-${_realname}"
pkgver=7.9000
pkgrel=1
pkgdesc="An image viewing/manipulation program (mingw-w64)"
arch=('any')
mingw_arch=('mingw32' 'mingw64' 'ucrt64' 'clang64')
url="https://www.imagemagick.org/"
license=("custom")
makedepends=("${MINGW_PACKAGE_PREFIX}-cairo"
             "${MINGW_PACKAGE_PREFIX}-gcc"
             "${MINGW_PACKAGE_PREFIX}-ghostscript"
             "${MINGW_PACKAGE_PREFIX}-graphviz"
             "${MINGW_PACKAGE_PREFIX}-librsvg"
             "${MINGW_PACKAGE_PREFIX}-libxml2"
             "${MINGW_PACKAGE_PREFIX}-openexr"
             "${MINGW_PACKAGE_PREFIX}-pango"
             "${MINGW_PACKAGE_PREFIX}-pkg-config")
depends=("${MINGW_PACKAGE_PREFIX}-bzip2"
         "${MINGW_PACKAGE_PREFIX}-djvulibre"
         "${MINGW_PACKAGE_PREFIX}-flif"
         "${MINGW_PACKAGE_PREFIX}-fftw"
         "${MINGW_PACKAGE_PREFIX}-fontconfig"
         "${MINGW_PACKAGE_PREFIX}-freetype"
         "${MINGW_PACKAGE_PREFIX}-glib2"
         "${MINGW_PACKAGE_PREFIX}-gsfonts"
         "${MINGW_PACKAGE_PREFIX}-jasper"
         "${MINGW_PACKAGE_PREFIX}-jbigkit"
         "${MINGW_PACKAGE_PREFIX}-lcms2"
         "${MINGW_PACKAGE_PREFIX}-libheif"
         "${MINGW_PACKAGE_PREFIX}-liblqr"
         "${MINGW_PACKAGE_PREFIX}-libpng"
         "${MINGW_PACKAGE_PREFIX}-libraqm"
         "${MINGW_PACKAGE_PREFIX}-libraw"
         "${MINGW_PACKAGE_PREFIX}-libtiff"
         "${MINGW_PACKAGE_PREFIX}-libtool"
         "${MINGW_PACKAGE_PREFIX}-libwebp"
         "${MINGW_PACKAGE_PREFIX}-libwmf"
         "${MINGW_PACKAGE_PREFIX}-libxml2"
         "${MINGW_PACKAGE_PREFIX}-openjpeg2"
         "${MINGW_PACKAGE_PREFIX}-ttf-dejavu"
         "${MINGW_PACKAGE_PREFIX}-xz"
         "${MINGW_PACKAGE_PREFIX}-zlib"
         "${MINGW_PACKAGE_PREFIX}-zstd")
optdepends=("${MINGW_PACKAGE_PREFIX}-ghostscript: for Ghostscript support"
            "${MINGW_PACKAGE_PREFIX}-openexr: for OpenEXR support"
            "${MINGW_PACKAGE_PREFIX}-librsvg: for SVG support"
            "${MINGW_PACKAGE_PREFIX}-graphviz: for GVC support")
options=('staticlibs' 'strip' '!debug' 'libtool')
source_dir="$GITHUB_WORKSPACE"
validpgpkeys=('D8272EF51DA223E4D05B466989AB63D48277377A')

prepare() {
  cd ${source_dir}
  # autoreconf -fi
}

build() {
  export lt_cv_deplibs_check_method='pass_all'
  [[ $CARCH = "i686" ]] && EXTRAOPTS="--with-gcc-arch=i686"
  [[ $CARCH = "x86_64" ]] && EXTRAOPTS="" #EXTRAOPTS="--with-gcc-arch=x86-64"

  [[ -d ${source_dir}/build-${MINGW_CHOST} ]] && rm -rf ${source_dir}/build-${MINGW_CHOST}
  mkdir -p ${source_dir}/build-${MINGW_CHOST} && cd ${source_dir}/build-${MINGW_CHOST}

  # See: https://github.com/msys2/MINGW-packages/blob/master/mingw-w64-imagemagick/ImageMagick-7.0.1.3-mingw.patch
  #export LIBS="-lws2_32"
  ../configure \
    --prefix=${MINGW_PREFIX} \
    --build=${MINGW_CHOST} \
    --host=${MINGW_CHOST} \
    --disable-deprecated \
    --enable-legacy-support \
    --enable-hdri \
    --with-djvu \
    --with-fftw \
    --with-gslib \
    --with-gvc \
    --with-flif \
    --with-lcms \
    --with-lqr \
    --with-modules \
    --with-openexr \
    --with-openjp2 \
    --with-rsvg \
    --with-webp \
    --with-wmf \
    --with-xml \
    --without-autotrace \
    --without-dps \
    --without-fpx \
    --with-jbig \
    --without-perl \
    --without-x \
    --with-raqm \
    --with-magick-plus-plus \
    --with-windows-font-dir=c:/Windows/fonts \
    --with-gs-font-dir=${MINGW_PREFIX}/share/fonts/gsfonts \
    --with-dejavu-font-dir=${MINGW_PREFIX}/share/fonts/TTF \
    $EXTRAOPTS \
    CFLAGS="${CFLAGS}" CPPFLAGS="${CPPFLAGS}" LDFLAGS="${LDFLAGS} -lws2_32"

  if check_option "debug" "y"; then
    MAKE_VERBOSE="V=1"
  fi
  make ${MAKE_VERBOSE}

    #--enable-opencl \
    #--with-perl-options="INSTALLDIRS=vendor"
}

package() {
  cd ${source_dir}/build-${MINGW_CHOST}
  if check_option "debug" "y"; then
    MAKE_VERBOSE="V=1"
  fi
  make -j1 DESTDIR="${pkgdir}" install ${MAKE_VERBOSE}

  #find . -name "*.xml" -exec sed -i "s/${MINGW_PREFIX}/newWord/g" '{}' \;
  install -Dm644 ${source_dir}/LICENSE "${pkgdir}${MINGW_PREFIX}/share/licenses/${_realname}/LICENSE"
  install -Dm644 ${source_dir}/NOTICE  "${pkgdir}${MINGW_PREFIX}/share/licenses/${_realname}/NOTICE"

  local PREFIX_WIN=$(cygpath -wm ${MINGW_PREFIX})
  # fix hard-coded pathes in XML files.
  find ${pkgdir}${MINGW_PREFIX}/lib -name "*.xml" -exec sed -e "s|${PREFIX_WIN}|${MINGW_PREFIX}|g" -i {} \;
  # fix libtool .la files
  find ${pkgdir}${MINGW_PREFIX}/lib -name "*.la" -exec sed -e "s|${PREFIX_WIN}|${MINGW_PREFIX}|g" -i {} \;
  # fix hard-coded pathes in .pc files
  for _f in "${pkgdir}${MINGW_PREFIX}"\/lib\/pkgconfig\/*.pc; do
    sed -e "s|${PREFIX_WIN}|${MINGW_PREFIX}|g" -i ${_f}
  done
}

check() {
  cd "${source_dir}/build-${MINGW_CHOST}"
  #make -j1 check || true
}
