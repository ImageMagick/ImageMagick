#include <iostream>
#include <list>
#include <algorithm>

#include <Magick++/Image.h>
#include <Magick++/STL.h>

static std::string getInitializer(const std::string magick_module)
{
  if ((magick_module == "BGR") || (magick_module == "CMYK") || (magick_module =="RGB") || (magick_module =="YUV"))
    return "interlace";
  if (magick_module == "PNG")
    return "png";
  return "";
}

int main() {
  std::list<Magick::CoderInfo> coderList;
  coderInfoList(&coderList, Magick::CoderInfo::TrueMatch, Magick::CoderInfo::AnyMatch, Magick::CoderInfo::AnyMatch);

  std::list<std::string> allowedNames;
  allowedNames.push_back("BGR");
  allowedNames.push_back("BMP");
  allowedNames.push_back("CMYK");
  allowedNames.push_back("DDS");
  allowedNames.push_back("EPT");
  allowedNames.push_back("FAX");
  allowedNames.push_back("HTML");
  allowedNames.push_back("JP2");
  allowedNames.push_back("JPEG");
  allowedNames.push_back("PCD");
  allowedNames.push_back("PCD");
  allowedNames.push_back("PDF");
  allowedNames.push_back("PNG");
  allowedNames.push_back("PS");
  allowedNames.push_back("PS2");
  allowedNames.push_back("PS3");
  allowedNames.push_back("RGB");
  allowedNames.push_back("SVG");
  allowedNames.push_back("TIFF");
  allowedNames.push_back("TXT");
  allowedNames.push_back("YCBCR");

  std::list<std::string> excludeList;
  excludeList.push_back("GRADIENT");
  excludeList.push_back("LABEL");
  excludeList.push_back("NULL");
  excludeList.push_back("PATTERN");
  excludeList.push_back("PLASMA");
  excludeList.push_back("SCREENSHOT");
  excludeList.push_back("TXT");
  excludeList.push_back("XC");

  for (std::list<Magick::CoderInfo>::iterator it = coderList.begin(); it != coderList.end(); it++)
  {
    std::string module=(*it).module();
    if (std::find(excludeList.begin(), excludeList.end(), module) != excludeList.end())
      continue;

    if ((*it).name() == module)
      std::cout << ((*it).isWritable() ? "+" : "-") << module << ":" << getInitializer(module) << std::endl;
    else if (std::find(allowedNames.begin(), allowedNames.end(), module) != allowedNames.end())
      std::cout << ((*it).isWritable() ? "+" : "-") << (*it).name() << ":" << getInitializer(module) << std::endl;
  }
}
