#include <iostream>
#include <list>
#include <algorithm>

#include <Magick++/Image.h>
#include <Magick++/STL.h>

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
  allowedNames.push_back("GRADIENT");
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

  for (std::list<Magick::CoderInfo>::iterator it = coderList.begin(); it != coderList.end(); it++)
  {
    if ((*it).name() == (*it).module())
      std::cout << ((*it).isWritable() ? "+" : "-") << (*it).module() << std::endl;
    else if (std::find(allowedNames.begin(), allowedNames.end(), (*it).module()) != allowedNames.end())
      std::cout << ((*it).isWritable() ? "+" : "-") << (*it).name() << std::endl;
  }
}
