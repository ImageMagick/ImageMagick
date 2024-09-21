class HomebrewImagemagick < Formula
  desc "Home Brewed version of ImageMagick with display delegates"
  homepage "https://github.com/wdongw/homebrew-ImageMagick/releases"
  url "https://github.com/wdongw/homebrew-ImageMagick/releases/download/X11/imagemagick_7.11-38.zip"
  sha256 "9e18354d5f1baf94e94d70a14fe8ef702b4ce505916c82acaff2a478f20d3790"
  license "ImageMagick"
  revision 1
  version "7.11-38"
  head "https://github.com/wdongw/homebrew-ImageMagick.git", branch: "main"
  
  bottle do
    sha256 arm64_sequoia: "c2cb0b528e9d2265cd80df802438f8c42826c0f394c4862720082a4655061ead"
    sha256 arm64_sonoma:  "0f83ef549c139885ac25385a001d9a9d6de78a49ac00c6b0d554857fede02159"
    sha256 arm64_ventura: "0fb63cab295985d97f12b4244149a268f11c9fc73fa7ffd03595c7f7133a03f1"
    sha256 sonoma:        "9e18354d5f1baf94e94d70a14fe8ef702b4ce505916c82acaff2a478f20d3790"
    sha256 ventura:       "9e18354d5f1baf94e94d70a14fe8ef702b4ce505916c82acaff2a478f20d3790"
    sha256 sequoia:       "9e18354d5f1baf94e94d70a14fe8ef702b4ce505916c82acaff2a478f20d3790"
    sha256 x86_64_linux:  "76d2634d9692e27eb767449178d06999f6863d43e385317f5b3ee37f9c9852f4"
  end

  depends_on "pkg-config" => :build
  depends_on "fontconfig"
  depends_on "freetype"
  depends_on "ghostscript"
  depends_on "jpeg-turbo"
  depends_on "jpeg-xl"
  depends_on "libheif"
  depends_on "liblqr"
  depends_on "libpng"
  depends_on "libraw"
  depends_on "libtiff"
  depends_on "libtool"
  depends_on "little-cms2"
  depends_on "openexr"
  depends_on "openjpeg"
  depends_on "webp"
  depends_on "xz"

  uses_from_macos "bzip2"
  uses_from_macos "libxml2"
  uses_from_macos "zlib"

  on_macos do
    depends_on "gettext"
    depends_on "glib"
    depends_on "imath"
    depends_on "libomp"
  end

  on_linux do
    depends_on "libx11"
  end

  skip_clean :la

  def install
    # Avoid references to shim
    inreplace Dir["**/*-config.in"], "@PKG_CONFIG@", Formula["pkg-config"].opt_bin/"pkg-config"
    # versioned stuff in main tree is pointless for us
    inreplace "configure", "${PACKAGE_NAME}-${PACKAGE_BASE_VERSION}", "${PACKAGE_NAME}"

    args = [
      "--enable-osx-universal-binary=no",
      "--disable-silent-rules",
      "--disable-opencl",
      "--enable-shared",
      "--enable-static",
      "--with-freetype=yes",
      "--with-gvc=no",
      "--with-modules",
      "--with-openjp2",
      "--with-openexr",
      "--with-webp=yes",
      "--with-heic=yes",
      "--with-raw=yes",
      "--with-gslib",
      "--with-gs-font-dir=#{HOMEBREW_PREFIX}/share/ghostscript/fonts",
      "--with-lqr",
      "--without-djvu",
      "--without-fftw",
      "--without-pango",
      "--without-wmf",
      "--enable-openmp",
      "--with-x",
    ]
    if OS.mac?
      args += [
        #"--without-x",
        # Work around "checking for clang option to support OpenMP... unsupported"
        "ac_cv_prog_c_openmp=-Xpreprocessor -fopenmp",
        "ac_cv_prog_cxx_openmp=-Xpreprocessor -fopenmp",
        "LDFLAGS=-lomp -lz",
      ]
    end

    system "./configure", *std_configure_args, *args
    system "make", "install"
  end

  test do
    assert_match "PNG", shell_output("#{bin}/identify #{test_fixtures("test.png")}")

    # Check support for recommended features and delegates.
    features = shell_output("#{bin}/magick -version")
    %w[Modules freetype heic jpeg png raw tiff].each do |feature|
      assert_match feature, features
    end

    # Check support for a few specific image formats, mostly to ensure LibRaw linked correctly.
    formats = shell_output("#{bin}/magick -list format")
    ["AVIF  HEIC      rw+", "ARW  DNG       r--", "DNG  DNG       r--"].each do |format|
      assert_match format, formats
    end
    assert_match "Helvetica", shell_output("#{bin}/magick -list font")
  end
end
