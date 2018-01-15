#include <iostream>
#include <list>

#include <Magick++/Image.h>
#include <Magick++/STL.h>

int main() {
  std::list<Magick::CoderInfo> coderList;
  coderInfoList(&coderList, Magick::CoderInfo::TrueMatch, Magick::CoderInfo::AnyMatch, Magick::CoderInfo::AnyMatch);

  for (std::list<Magick::CoderInfo>::iterator it = coderList.begin(); it != coderList.end(); it++)
  {
    //std::cout << ((*it).isWritable() ? "+" : "-") << (*it).name() << std::endl;
    std::cout << (*it).name() << std::endl;
  }
}
