// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002, 2003
//
// Test Magick::Color classes
//

#include <Magick++.h>
#include <string>
#include <iostream>

using namespace std;

using namespace Magick;

int main( int /*argc*/, char **argv)
{

  // Initialize ImageMagick install location for Windows
  InitializeMagick(*argv);

  int failures=0;

  try {

    //
    // Verify conversion from named colors as well as ColorRGB constructor
    //

    {
      struct colorStr
      {
	const char* color;
	double red;
	double green;
	double blue;
      };

      // Convert ratios from rgb.txt via value/255
      struct colorStr colorMap [] =
      { 
	{ "red", 1,0,0 },
	{ "green", 0,0.5019607843137256,0 },
	{ "blue", 0,0,1 },
	{ "black", 0,0,0 },
	{ "white", 1,1,1 },
	{ "cyan", 0,1,1 },
	{ "magenta", 1,0,1 },
	{ "yellow", 1,1,0 },
	{ NULL, 0,0,0 }
      };

      for ( int i = 0; colorMap[i].color != NULL; i++ )
	{
	  {
	    Color color( colorMap[i].color );
	    ColorRGB colorMatch( colorMap[i].red,
				 colorMap[i].green,
				 colorMap[i].blue );
	    if ( color != colorMatch )
	      {
		++failures;
		cout << "Line: " << __LINE__ << " Color(\""
		     << colorMap[i].color << "\") is "
		     << string(color)
		     << " rather than "
		     << string(colorMatch)
		     << endl;
                // printf ("Green: %10.16f\n", color.green());
	      }
	  }
	}      
    }

    // Test conversion to/from X11-style color specifications
    {
      const char * colorStrings[] =
      {
	"#ABC",
	"#AABBCC",
	"#AAAABBBBCCCC",
	NULL
      };

#if MAGICKCORE_QUANTUM_DEPTH == 8
      string expectedString = "#AABBCC";
#elif MAGICKCORE_QUANTUM_DEPTH == 16
      string expectedString = "#AAAABBBBCCCC";
#elif MAGICKCORE_QUANTUM_DEPTH == 32
      string expectedString = "#AAAAAAAABBBBBBBBCCCCCCCC";
#elif MAGICKCORE_QUANTUM_DEPTH == 64
      string expectedString = "#AAAAAAAAAAAAAAAABBBBBBBBBBBBBBBBCCCCCCCCCCCCCCCC";
#else
# error Quantum depth not supported!
#endif

      for ( int i = 0; colorStrings[i] != NULL; ++i )
	{
	  if ( string(Color(colorStrings[i])) != expectedString )
	    {
	      ++failures;
	      cout << "Line: " << __LINE__
		   << " Conversion from " << colorStrings[i]
		   << " is "
		   << string(Color(colorStrings[i])) << " rather than "
		   << expectedString
		   << endl;
	    }
	}
    }

    // Test ColorGray
    {
      double resolution = 1.0/QuantumRange;
      if ( resolution < 0.0000001 )
        resolution = 0.0000001;
      double max_error = resolution + MagickEpsilon;

      for( double value = 0; value < 1.0 + MagickEpsilon; value += resolution )
        {
          ColorGray gray(value);
          if ( gray.shade() < value - max_error || gray.shade() > value + max_error )
            {
              ++failures;
              cout << "Line: " << __LINE__
                   << " shade is "
                   << gray.shade()
                   << " rather than nominal "
                   << value
                   << endl;
            }
        }
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
