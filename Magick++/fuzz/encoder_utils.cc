static int fuzzEncoderWithStringFilename(const std::string encoder, const uint8_t *Data, size_t Size)
{
  // Allow a bit extra to make sure we do proper bounds checking in Magick++
  if (Size > MagickPathExtent)
    return 0;
  std::string color(reinterpret_cast<const char*>(Data), Size);

  Magick::Image image;
  try {
    image.read(encoder + ":" + color);
  }
  catch (Magick::Exception &e) {
  }
  return 0;
}