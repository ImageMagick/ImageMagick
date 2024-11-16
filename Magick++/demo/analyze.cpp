//
// Demonstrate using the 'analyze' process module to compute
// image statistics.
//
// Copyright Bob Friesenhahn, 2003, 2004
//
// Copyright @ 2013 ImageMagick Studio LLC, a non-profit organization
// dedicated to making software imaging solutions freely available.
//
// Usage: analyze file...
//

#include <Magick++.h>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <list>
using namespace std; 
using namespace Magick;

int main(int argc,char **argv) 
{
  if ( argc < 2 )
    {
      cout << "Usage: " << argv[0] << " file..." << endl;
      exit( 1 );
    }

  // Initialize ImageMagick install location for Windows
  MagickPlusPlusGenesis genesis(*argv);

  {
    std::list<std::string> attributes;

    attributes.push_back("TopLeftColor");
    attributes.push_back("TopRightColor");
    attributes.push_back("BottomLeftColor");
    attributes.push_back("BottomRightColor");
    attributes.push_back("filter:brightness:mean");
    attributes.push_back("filter:brightness:standard-deviation");
    attributes.push_back("filter:brightness:kurtosis");
    attributes.push_back("filter:brightness:skewness");
    attributes.push_back("filter:saturation:mean");
    attributes.push_back("filter:saturation:standard-deviation");
    attributes.push_back("filter:saturation:kurtosis");
    attributes.push_back("filter:saturation:skewness");

    char **arg = &argv[1];
    while ( *arg )
      {
        string fname(*arg);
        try {
          cout << "File: " << fname << endl;
          Image image( fname );

          /* Analyze module does not require an argument list */
          image.process("analyze",0,0);

          list<std::string>::iterator pos = attributes.begin();
          while(pos != attributes.end())
            {
              cout << "  " << setw(16) << setfill(' ') << setiosflags(ios::left)
                   << *pos << " = " << image.attribute(*pos) << endl;
              pos++;
            }
        }
        catch( Exception &error_ ) 
          { 
            cout << error_.what() << endl; 
          }
        ++arg;
      }
  }

  return 0; 
}
