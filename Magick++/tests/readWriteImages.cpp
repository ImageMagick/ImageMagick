// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2003
//
// Test STL readImages and writeImages functions
//

#include <Magick++.h>
#include <string>
#include <iostream>
#include <list>
#include <vector>

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

    //
    // Test readImages and writeImages
    //
      
    list<Image> first;
    readImages( &first, srcdir + "test_image_anim.miff" );
      
    if ( first.size() != 6 )
      {
	++failures;
	cout << "Line: " << __LINE__ 
	     << "  Read images failed, number of frames is "
	     << first.size()
	     << " rather than 6 as expected." << endl;
      }
      
    writeImages( first.begin(), first.end(), "testmagick_anim_out.miff" );
      
    list<Image> second;
    readImages( &second, "testmagick_anim_out.miff" );
      
    list<Image>::iterator firstIter = first.begin();
    list<Image>::iterator secondIter = second.begin();
    while( firstIter != first.end() && secondIter != second.end() )
      {

	if ( firstIter->scene() != secondIter->scene() )
	  {
	    ++failures;
	    cout << "Line: " << __LINE__ 
		 << "  Image scene: " << secondIter->scene()
		 << " is not equal to original "
		 << firstIter->scene()
		 << endl;
	  }

	if ( firstIter->rows() != secondIter->rows() )
	  {
	    ++failures;
	    cout << "Line: " << __LINE__ 
		 << "  Image rows " << secondIter->rows()
		 << " are not equal to original "
		 << firstIter->rows()
		 << endl;
	  }

	if ( firstIter->columns() != secondIter->columns() )
	  {
	    ++failures;
	    cout << "Line: " << __LINE__ 
		 << "  Image columns " << secondIter->columns()
		 << " are not equal to original "
		 << firstIter->rows()
		 << endl;
	  }

	firstIter++;
	secondIter++;
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

