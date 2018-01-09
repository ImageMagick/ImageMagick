#include <iostream>

#include <Magick++/Blob.h>
#include <Magick++/Image.h>

extern "C" int main() {
  size_t nFormats;
  Magick::ExceptionInfo ex;
  const Magick::MagickInfo **formats = GetMagickInfoList("*", &nFormats, &ex);

  for (size_t i = 0; i < nFormats; i++) {
    const Magick::MagickInfo *format = formats[i];
    if (format->encoder && format->name) {
      std::cout << format->name << std::endl;
    }
  }
}
