static int fuzzEncoderWithStringFilename(const std::string encoder, const uint8_t *Data, size_t Size, bool (*validate)(const std::string &) = NULL)
{
  // Allow a bit extra to make sure we do proper bounds checking in Magick++
  if (Size > MagickPathExtent)
    return 0;

  std::string fileName(reinterpret_cast<const char*>(Data), Size);

  // Can be used to deny specific file names
  if ((validate != NULL) && (validate(fileName) == false))
    return 0;

  Magick::Image image;
  try {
    image.read(encoder + ":" + fileName);
  }
  catch (Magick::Exception &e) {
  }
  return 0;
}