// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 2003
//
// Test STL colorHistogram function
//

#undef USE_VECTOR
#define USE_MAP

#include <Magick++.h>
#include <string>
#include <iostream>
#include <iomanip>
#if defined(USE_VECTOR)
#  include <vector>
#  include <utility>
#endif
#if defined(USE_MAP)
#  include <map>
#endif

using namespace std;

using namespace Magick;

int main( int /*argc*/, char ** argv)
{

  // Initialize ImageMagick install location for Windows
  InitializeMagick(*argv);

  int failures=0;

  try {

    string srcdir("");
    if(getenv("SRCDIR") != 0)
      srcdir = getenv("SRCDIR");

    // Read image
    Image image;
    image.read( srcdir + "test_image.miff" );

    // Create histogram vector
#if defined(USE_MAP)
    std::map<Color,size_t> histogram;
#elif defined(USE_VECTOR)
    std::vector<std::pair<Color,size_t> > histogram;
#endif

    colorHistogram( &histogram, image );

    // Print out histogram
#if (MAGICKCORE_QUANTUM_DEPTH == 8)
    int quantum_width=3;
#elif (MAGICKCORE_QUANTUM_DEPTH == 16)
    int quantum_width=5;
#else
    int quantum_width=10;
#endif

    cout << "Histogram for file \"" << image.fileName() << "\"" << endl
         << histogram.size() << " entries:" << endl;

#if defined(USE_MAP)
    std::map<Color,size_t>::const_iterator p=histogram.begin();
#elif defined(USE_VECTOR)
    std::vector<std::pair<Color,size_t> >::const_iterator p=histogram.begin();
#endif
    while (p != histogram.end())
      {
        cout << setw(10) << (int)p->second << ": ("
             << setw(quantum_width) << (int)p->first.redQuantum() << ","
             << setw(quantum_width) << (int)p->first.greenQuantum() << ","
             << setw(quantum_width) << (int)p->first.blueQuantum() << ")"
             << endl;
        p++;
      }
  }

  catch( Exception &error_ )
    {
      cout << "Caught exception: " << error_.what() << endl;
      return 1;
    }
  catch( exception &error_ )
    {
      cout << "Caught exception: " << error_.what() << endl;
      return 1;
    }

  if ( failures )
    {
      cout << failures << " failures" << endl;
      return 1;
    }
  
  return 0;
}

