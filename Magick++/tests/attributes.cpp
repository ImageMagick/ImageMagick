// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002, 2003
//
// Tests for setting/getting Magick::Image attributes
//

#include <Magick++.h>
#include <string>
#include <iostream>

using namespace std;

using namespace Magick;

int main( int /*argc*/, char ** argv)
{

  // Initialize ImageMagick install location for Windows
  InitializeMagick(*argv);

  int failures=0;

  try {

    unsigned int columns = 640;
    unsigned int rows = 480;
    Geometry geometry(columns,rows);
    Color canvasColor( "red" );
    Image image( geometry, canvasColor);

    //
    // antiAlias
    //

    // Test default value
    if ( image.antiAlias() != true )
      {
	++failures;
	cout << "Line: " << __LINE__ << ", antiAlias default not true" << endl;
      }

    // Test setting false
    image.antiAlias( false );
    if ( image.antiAlias() != false )
      {
	++failures;
	cout << "Line: " << __LINE__ << ", antiAlias not false" << endl;
      }

    // Test setting true
    image.antiAlias( true );
    if ( image.antiAlias() != true )
      {
	++failures;
	cout << "Line: " << __LINE__ << ", antiAlias not true" << endl;
      }

    //
    // adjoin
    //

    // Test default value
    if ( image.adjoin() != true )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", adjoin default not 'true' as expected" << endl;
      }

    // Test setting false
    image.adjoin( false );
    if ( image.adjoin() != false )
      {
	++failures;
	cout << "Line: " << __LINE__ << ", adjoin failed set to 'false'" << endl;
      }

    // Test setting true
    image.adjoin( true );
    if ( image.adjoin() != true )
      {
	++failures;
	cout << "Line: " << __LINE__ << ", adjoin failed set to 'true'" << endl;
      }

    //
    // animationDelay
    //

    // Test default value
    if ( image.animationDelay() != 0 )
      {
	++failures;
	cout << "Line: " << __LINE__ << ", animationDelay default ("
	     << image.animationDelay()
	     << ") not 0 as expected" << endl;
      }

    // Test setting to 0
    image.animationDelay( 0 );
    if ( image.animationDelay() != 0 )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", failed to set animationDelay to 0" << endl;
      }

    // Test setting to 100
    image.animationDelay( 100 );
    if ( image.animationDelay() != 100 )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", failed to set animationDelay to 100" << endl;
      }
    image.animationDelay(0);

    //
    // animationIterations
    //

    // Test default value
    if ( image.animationIterations() != 0 )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", animationIterations default ("
	     << image.animationIterations()
	     << ") not 0 as expected" << endl;
      }

    // Test setting to 0
    image.animationIterations( 0 );
    if ( image.animationIterations() != 0 )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", failed to set animationIterations to 0" << endl;
      }

    // Test setting to 100
    image.animationIterations( 100 );
    if ( image.animationIterations() != 100 )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", failed to set animationIterations to 100" << endl;
      }
    image.animationIterations( 0 );

    //
    // backgroundColor
    //

    // Test default value.
    if ( image.backgroundColor() != ColorRGB("white") )
      {
	++failures;
	cout << "Line: " << __LINE__ << ", backgroundColor default ("
	     << string(image.backgroundColor())
	     << ") is incorrect" << endl;
      }

    // Test setting to blue
    image.backgroundColor("blue");
    if ( !image.backgroundColor().isValid() )
      {
	++failures;
	cout << "Line: " << __LINE__ << ", backgroundColor ("
	     << string(image.backgroundColor())
	     << ") failed set to 'blue'" << endl;
      }
    else
      if ( string(image.backgroundColor()) != "#0000FF" &&
	   string(image.backgroundColor()) != "#00000000FFFF" &&
	   string(image.backgroundColor()) != "#0000000000000000FFFFFFFF" )
	{
	  ++failures;
	  cout << "Line: " << __LINE__ << ", backgroundColor ("
	       <<  string(image.backgroundColor()) << ") is incorrect"
	       << endl;
	}

    // Test setting using hex color
    image.backgroundColor("#00AAFF");
    if ( !image.backgroundColor().isValid() )
      {
	++failures;
	cout << "Line: " << __LINE__ << ", backgroundColor ("
	     << string(image.backgroundColor())
	     << ") is incorrectly invalid" << endl;
      }
    else
      if ( string(image.backgroundColor()) != "#00AAFF" && 
	   string(image.backgroundColor()) != "#0000AAAAFFFF" && 
	   string(image.backgroundColor()) != "#00000000AAAAAAAAFFFFFFFF" )
	{
	  ++failures;
	  cout << "Line: " << __LINE__
	       << ", backgroundColor ("
	       << string(image.backgroundColor())
	       << ") is incorrect"
	       << endl;
	}

    //
    // backgroundTexture
    //

    // Test default value
    if ( image.backgroundTexture() != "" )
      {
	++failures;
	cout << "Line: " << __LINE__ << ", backgroundTexture default ("
	     << image.backgroundTexture()
	     << ") is incorrect" << endl;
      }

    // Test setting/getting value
    image.backgroundTexture("afile.jpg");
    if ( image.backgroundTexture() != "afile.jpg" )
      {
	++failures;
	cout << "Line: " << __LINE__ << ", backgroundTexture ("
	     << image.backgroundTexture()
	     << ") is incorrect" << endl;
      }

    // Test setting back to default
    image.backgroundTexture("");
    if ( image.backgroundTexture() != "" )
      {
	++failures;
	cout << "Line: " << __LINE__
	     << ", backgroundTexture ("
	     << image.backgroundTexture()
	     << ") failed to set to \"\"" << endl;
      }

    //
    // baseColumns
    //
    if ( image.baseColumns() != columns )
      {
	++failures;
	cout << "Line: " << __LINE__
	     << ", baseColumns ("
	     << image.baseColumns()
	     << ") is not equal to "
	     << columns
	     << " as expected"
	     << endl;
      }


    //
    // baseFilename
    //
    // Base filename is color for xc images
    if ( image.baseFilename() != "xc:#FF0000" &&
	 image.baseFilename() != "xc:#FFFF00000000" &&
	 image.baseFilename() != "xc:#FFFFFFFF0000000000000000")
      {
	++failures;
	cout << "Line: " << __LINE__
	     << ", baseFilename ("
	     << image.baseFilename()
	     << ") is incorrect"
	     << endl;
      }

    //
    // baseRows
    //
    if ( image.baseRows() != rows )
      {
	++failures;
	cout << "Line: " << __LINE__
	     << ", baseRows ("
	     << image.baseRows()
	     << ") != rows ("
	     << rows
	     << ")"
	     << endl;
      }

    //
    // borderColor
    //
    if ( image.borderColor() != ColorRGB("#dfdfdf") )
      {
	++failures;
	cout << "Line: " << __LINE__
	     << ",  borderColor default ("
	     << string(image.borderColor())
	     << ") is incorrect" << endl;
      }

    image.borderColor("#FF0000");
    if ( image.borderColor() != Color("#FF0000") )
      {
	++failures;
	cout << "Line: " << __LINE__
	     << ", failed to set borderColor ("
	     << string(image.borderColor())
	     << ")" << endl;
      }

    image.borderColor("black");
    if ( image.borderColor() != Color("#000000") )
      {
	++failures;
	cout << "Line: " << __LINE__
	     << ", failed to set borderColor ("
	     << string(image.borderColor())
	     << ")"
	     << endl;
      }
    
    //
    // boxColor
    //
    image.boxColor("#FF0000");
    if ( image.boxColor() != Color("#FF0000") )
      {
	++failures;
	cout << "Line: " << __LINE__
	     << ", failed to set boxColor ("
	     << string(image.boxColor())
	     << ")"
	     << endl;
      }

    image.boxColor("black");
    if ( image.boxColor() != Color("#000000") )
      {
	++failures;
	cout << "Line: " << __LINE__
	     << ", failed to set boxColor ("
	     << string(image.boxColor())
	     << ") to #000000"
	     << endl;
      }

    //
    // chromaBluePrimary
    //
    {
      // Test default setting
      double x, y;
      image.chromaBluePrimary( &x, &y );
      if ( x != 0 || y != 0 )
	{
	  ++failures;
	  cout << "Line: " << __LINE__
	       << ",  chromaBluePrimary x/y defaults are non-zero"
	       << endl;
	}

      // Test set/get
      image.chromaBluePrimary( 50, 100 );
      image.chromaBluePrimary( &x, &y );
      if ( x != 50 || y != 100 )
	{
	  ++failures;
	  cout << "Line: " << __LINE__
               << ",  chromaBluePrimary x/y failed set/get" << endl;
	}
    }

    //
    // chromaGreenPrimary
    //
    {
      // Test default setting
      double x, y;
      image.chromaGreenPrimary( &x, &y );
      if ( x != 0 || y != 0 )
	{
	  ++failures;
	  cout << "Line: " << __LINE__
               << ",  chromaGreenPrimary x/y defaults are non-zero" << endl;
	}

      // Test set/get
      image.chromaGreenPrimary( 50, 100 );
      image.chromaGreenPrimary( &x, &y );
      if ( x != 50 || y != 100 )
	{
	  ++failures;
	  cout << "Line: " << __LINE__
               << ",  chromaGreenPrimary x/y failed set/get" << endl;
	}
    }

    //
    // chromaRedPrimary
    //
    {
      // Test default setting
      double x, y;
      image.chromaRedPrimary( &x, &y );
      if ( x != 0 || y != 0 )
	{
	  ++failures;
	  cout << "Line: " << __LINE__
               << ",  chromaRedPrimary x/y defaults are non-zero" << endl;
	}

      // Test set/get
      image.chromaRedPrimary( 50, 100 );
      image.chromaRedPrimary( &x, &y );
      if ( x != 50 || y != 100 )
	{
	  ++failures;
	  cout << "Line: " << __LINE__
               << ",  chromaRedPrimary x/y failed set/get" << endl;
	}
    }

    //
    // chromaWhitePoint
    //
    {
      // Test default setting
      double x, y;
      image.chromaWhitePoint( &x, &y );
      if ( x != 0 || y != 0 )
	{
	  ++failures;
	  cout << "Line: " << __LINE__
               << ",  chromaWhitePoint x/y defaults are non-zero" << endl;
	}

      // Test set/get
      image.chromaWhitePoint( 50, 100 );
      image.chromaWhitePoint( &x, &y );
      if ( x != 50 || y != 100 )
	{
	  ++failures;
	  cout << "Line: " << __LINE__
               << ",  chromaWhitePoint x/y failed set/get" << endl;
	}
    }

    //
    // classType
    //
    if ( image.classType() != DirectClass )
      {
	++failures;
	cout << "Line: " << __LINE__ << ", classType is not DirectClass" << endl;
      }

    //
    // colorFuzz
    //

    // Test default
    if ( image.colorFuzz() != 0 )
      {
	++failures;
	cout << "Line: " << __LINE__ << ",  colorFuzz default is non-zero" << endl;
      }

    // Test set/get
    image.colorFuzz( 2 );
    if ( image.colorFuzz() != 2 )
      {
	++failures;
	cout << "Line: " << __LINE__ << ",  colorFuzz failed to set/get" << endl;
      }
    image.colorFuzz( 0 );

    //
    // columns
    //
    if ( image.columns() != columns )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", columns is not equal to canvas image columns" << endl;
      }

    //
    // comment
    //
    // Test default
    if ( image.comment().length() != 0 )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", comment default non-zero length" << endl;
      }

    // Test set/get
    {
      std::string comment("This is a comment.");
      image.comment( comment );
      if ( image.comment() != comment )
	{
	  ++failures;
	  cout << "Line: " << __LINE__ << ", comment set/get failed" << endl;
	}
    }

    // Test resetting comment
    image.comment( string() );
    if ( image.comment().length() != 0 )
      {
	++failures;
	cout << "Line: " << __LINE__ << ", comment failed to reset" << endl;
      }

    //
    // compressType
    //
    // Test default
    if ( image.compressType() != UndefinedCompression )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", compressType default is incorrect" << endl;
      }

    // Test set/get
    image.compressType(RLECompression);
    if ( image.compressType() != RLECompression )
      {
	++failures;
	cout << "Line: " << __LINE__ << ", compressType set/get failed" << endl;
      }
    image.compressType(UndefinedCompression);

    //
    // density
    //
    {
      // Test defaults
      if ( image.density() != Geometry(72,72) )
	{
	  ++failures;
	  cout << "Line: " << __LINE__
               << ", density default is not 72x72 as expected" << endl;
	}
      
      // Test set/get
      Geometry density(150,75);
      image.density(density);
      if ( image.density() != density )
	{
	  ++failures;
	  cout << "Line: " << __LINE__ << ", density set/get failed" << endl;
	}


      if ( image.xResolution() != 150 ||
	   image.yResolution() != 75 )
	{
	  ++failures;
	  cout << "Line: " << __LINE__ << ", density set/get failed" << endl;
	}

      image.density("72x72");

    }

    //
    // Format specific defines
    //
    if (image.defineSet("foo","bar"))
      {
        ++failures;
        cout << "Line: " << __LINE__
             << ", define for foo:bar incorrectly reports set."
             << endl;
      }

    image.defineSet("foo","bar",true);
    if (!image.defineSet("foo","bar"))
      {
        ++failures;
        cout << "Line: " << __LINE__
             << ", define for foo:bar incorrectly reports not set."
             << endl;
      }

    image.defineSet("foo","bar",false);
    if (image.defineSet("foo","bar"))
      {
        ++failures;
        cout << "Line: " << __LINE__
             << ", define for foo:bar incorrectly reports set."
             << endl;
      }

    image.defineValue("foo","bar","value");
    std::string value = image.defineValue("foo","bar");
    if (image.defineValue("foo","bar") != "value")
      {
        ++failures;
        cout << "Line: " << __LINE__
             << ", define for foo:bar incorrectly reports value \""
             << value << "\""
             << endl;
      }

    image.defineSet("foo","bar",false);
    if (image.defineSet("foo","bar"))
      {
        ++failures;
        cout << "Line: " << __LINE__
             << ", define for foo:bar incorrectly reports set."
             << endl;
      }

    //
    // depth
    //
    if ( image.depth() != MAGICKCORE_QUANTUM_DEPTH )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", depth ("
             << image.depth()
             << ") is not equal to " << MAGICKCORE_QUANTUM_DEPTH << endl;
      }

    //
    // Directory
    //
    {
      // Since this is not a montage image, simply verify error report
      bool caughtException = false;
      cout << "Testing throwing and catching exceptions. A program crash or a message" << endl
           << "that the exception was not caught indicates a test failure.  A properly" << endl
           << "formatted exception message indicates success:" << endl;
      try
	{
	  //image.directory();
          Magick::Image bad_image("foo");
	}
      catch ( Exception &exception_)
	{
          cout << "Caught exception, good!:" << endl
               << "  \"" << exception_.what() << "\"" << endl;
	  caughtException = true;
	}
      if ( caughtException != true )
	{
	  ++failures;
          cout << "failed to catch exception!" << endl;
	}
    }

    //
    // fileName
    //
    // Test default
    if ( image.fileName() != string("xc:") + string(canvasColor) )
      {
	++failures;
	cout << "Line: "
	     << __LINE__
	     << ", fileName ("
	     << image.fileName()
	     << ") is not canvas color ("
	     << string(canvasColor)
	     <<") as expected" << endl;
      }

    // Set/get value
    image.fileName("filename.jpg");
    if ( image.fileName() != "filename.jpg" )
      {
	++failures;
	cout << "Line: "
	     << __LINE__
	     << ", fileName ("
	     << image.fileName()
	     << ") failed to set/get" << endl;
      }
    image.fileName(canvasColor);

    //
    // fileSize
    //
    // Test default
    if ( image.fileSize() != 0 )
      {
	++failures;
	cout << "Line: " << __LINE__ << ", fileSize ("
	     << static_cast<long>(image.fileSize())
	     << ") is not zero as expected" << endl;
      }

    //
    // filterType
    //
    // Test default
    if ( image.filterType() != UndefinedFilter )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", filterType default ("
             << (int)image.filterType()
             << ") is incorrect" << endl;
      }

    // Test set/get
    image.filterType( TriangleFilter );
    if ( image.filterType() != TriangleFilter )
      {
	++failures;
	cout << "Line: " << __LINE__ << ", filterType set/get failed"
             << endl;
      }

    //
    // font
    //

    // Test set/get
    image.font("helvetica");
    if ( image.font() != "helvetica" )
      {
	++failures;
	cout << "Line: " << __LINE__ << ", font set/get failed" << endl;
      }
    // Test set to null font
    image.font( string() );
    if ( image.font().length() != 0 )
      {
	++failures;
	cout << "Line: " << __LINE__ << ", font failed to unset" << endl;
      }

    //
    // fontPointsize
    //
    // Test default
    if ( image.fontPointsize() != 0 )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", fontPointsize ("
             << image.fontPointsize()
             << ") is not default of 0 as expected"
             << endl;
      }

    // Test set/get
    image.fontPointsize(10);
    if ( image.fontPointsize() != 10 )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", fontPointsize set/get failed" << endl;
      }
    image.fontPointsize(12);

    //
    // format
    //
    if ( image.format() != "Constant image uniform color" )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", format (" << image.format() << ") is not expected value" << endl;
      }

    //
    // gamma
    //
    if ( image.gamma() != 0 )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", gamma correction is not zero as expected" << endl;
      }

    //
    // geometry
    //
    {
      bool caughtException = false;
      try
	{
	  image.geometry();
	}
      catch ( Exception )
	{
	  caughtException = true;
	}
      if ( caughtException != true )
	{
	  ++failures;
	  cout << "Line: " << __LINE__
               << ", geometry failed to report missing image geometry";
	}
    }

    //
    // gifDisposeMethod
    //
    // Test default
    if ( image.gifDisposeMethod() != 0 )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", gifDisposeMethod default is not zero as expected" << endl;
      }

    // Test set/get
    image.gifDisposeMethod(4);
    if ( image.gifDisposeMethod() != 4 )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", gifDisposeMethod set/get failed" << endl;
      }
    image.gifDisposeMethod(0);

    //
    // interlaceType
    //
    // Test default
    if ( image.interlaceType() != NoInterlace )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", interlaceType default is not NoInterlace as expected" << endl;
      }

    // Test set/get
    image.interlaceType( PlaneInterlace );
    if ( image.interlaceType() != PlaneInterlace )
      {
	++failures;
	cout << "Line: " << __LINE__ << ", interlaceType set/get failed" << endl;
      }
    image.interlaceType(NoInterlace);

    //
    // label
    //
    // Test default
    if ( image.label().length() != 0 )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", label default is not empty string as expected" << endl;
      }

    // Test set/get
    image.label("How now brown cow?");
    if ( image.label() != "How now brown cow?" )
      {
	++failures;
	cout << "Line: " << __LINE__ << ", label set/get failed" << endl;
      }
    // Test set to default
    image.label( string() );
    if ( image.label().length() != 0 )
      {
	++failures;
	cout << "Line: " << __LINE__ << ", label failed to unset" << endl;
      }

    //
    // lineWidth
    //
    // Test default
    if ( image.lineWidth() != 1 )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", lineWidth default is not 1 as expected" << endl;
      }

    // Test set/get
    image.lineWidth(2);
    if ( image.lineWidth() != 2 )
      {
	++failures;
	cout << "Line: " << __LINE__ << ", lineWidth set/get failed" << endl;
      }
    image.lineWidth(1);

    //
    // magick
    //
    // Test canvas default
    if ( image.magick() != "XC" )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", magick canvas default is not XC as expected" << endl;
      }

    // Test set/get
    image.magick("GIF");
    if ( image.magick() != "GIF" )
      {
	++failures;
	cout << "Line: " << __LINE__ << ", magick set/get failed" << endl;
      }

    image.magick("XC");

    //
    // matte
    //
    // Test default
    if ( image.matte() != false )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", matte default is not false as expected" << endl;
      }

    // Test set/get
    image.matte(true);
    if ( image.matte() != true )
      {
	++failures;
	cout << "Line: " << __LINE__ << ", matte set/get failed" << endl;
      }
    image.matte(false);

    //
    // matteColor
    //
    // Test default
    if ( image.matteColor() != Color("#BDBDBD") )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", matteColor default is not #BDBDBD as expected" << endl;
      }

    // Test set/get
    image.matteColor(ColorRGB(0.5,0.5,1));
    if ( image.matteColor() != ColorRGB(0.5,0.5,1) )
      {
	++failures;
	cout << "Line: " << __LINE__ << ", matteColor set/get failed" << endl;
      }

    // Test unset
    image.matteColor( Color() );

    image.matteColor("#BDBDBD");

    //
    // meanErrorPerPixel
    //
    if ( image.meanErrorPerPixel() != 0 )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", meanErrorPerPixel is not zero as expected" << endl;
      }

    //
    // montageGeometry
    //
    {
      bool caughtException = false;
      try
	{
	  image.montageGeometry();
	}
      catch ( Exception )
	{
	  caughtException = true;
	}
      if ( caughtException != true )
	{
	  ++failures;
	  cout << "Line: " << __LINE__
               << ", montageGeometry failed to report missing montage geometry";
	}
    }

    //
    // monochrome
    //
    // Test default
    if ( image.monochrome() != false )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", monochrome is not false as expected" << endl;
      }

    // Test set/get
    image.monochrome(true);
    if ( image.monochrome() != true )
      {
	++failures;
	cout << "Line: " << __LINE__ << ", monochrome get/set failed" << endl;
      }
    image.monochrome(false);

    //
    // normalizedMaxError
    //
    if ( image.normalizedMaxError() != 0 )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ",normalizedMaxError is not zero as expected" << endl;
      }

    //
    // normalizedMeanError
    //
    if ( image.normalizedMeanError() != 0 )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", normalizedMeanError is not zero as expected" << endl;
      }

    //
    // penColor
    //

    image.penColor(ColorRGB(0.5,0.5,1));
    if ( image.penColor() != ColorRGB(0.5,0.5,1) )
      {
	++failures;
	cout << "Line: " << __LINE__ << ", penColor ("
	     << string(image.penColor())
	     << ") set/get failed" << endl;
      }

    //
    // strokeColor
    //

    image.strokeColor(ColorRGB(0.5,0.5,1));
    if ( image.strokeColor() != ColorRGB(0.5,0.5,1) )
      {
	++failures;
	cout << "Line: " << __LINE__ << ", strokeColor ("
	     << string(image.strokeColor())
	     << ") set/get failed" << endl;
      }


    //
    // fillColor
    //

    image.fillColor(ColorRGB(0.5,0.5,1));
    if ( image.fillColor() != ColorRGB(0.5,0.5,1) )
      {
	++failures;
	cout << "Line: " << __LINE__ << ", fillColor ("
	     << string(image.fillColor())
	     << ") set/get failed" << endl;
      }

    //
    // pixelColor
    //
    // Test default
    if ( image.pixelColor(40,60) != canvasColor )
      {
	++failures;
	cout << "Line: " << __LINE__ << ", pixelColor default ("
	     << string(image.pixelColor(40,60))
	     << ") is not canvas color ("
	     << string(canvasColor)
	     << ") as expected" << endl;
      }

    // Test set/get
    image.pixelColor(40,60, ColorRGB(0.5,1,1));
    if ( image.pixelColor(40,60) != ColorRGB(0.5,1,1) )
      {
	++failures;
	cout << "Line: " << __LINE__ << ", pixelColor set/get failed" << endl;
      }

    //
    // page
    //
    // Test default
    if ( image.page() != Geometry(640,480,0,0) )
      {
	++failures;
	cout << "Line: " << __LINE__ << ", page default "
	     << "(" << string(image.page()) << ")"
	     << " is not 640x480 as expected" << endl;
      }

    // Test set/get
    image.page("letter+43+43>");
    if ( image.page() != "612x792+43+43" )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", page set/get failed (" << string(image.page()) << ")" << endl;
      }

    //
    // quality
    //
    // Test default
    if ( image.quality() != 0 )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", quality default is not 0 as expected" << endl;
      }

    // Test set/get
    image.quality(65);
    if ( image.quality() != 65 )
      {
	++failures;
	cout << "Line: " << __LINE__ << ", quality set/get failed" << endl;
      }
    image.quality(0);

    //
    // quantizeColors
    //
    // Test default
    if ( image.quantizeColors() != 256 )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", quantizeColors is not 256 as expected" << endl;
      }

    // Test set/get
    image.quantizeColors(200);
    if ( image.quantizeColors() != 200 )
      {
	++failures;
	cout << "Line: " << __LINE__ << ", quantizeColors set/get failed" << endl;
      }
    image.quantizeColors(0);

    //
    // quantizeColorSpace
    //
    // Test default
    if ( image.quantizeColorSpace() != UndefinedColorspace )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", quantizeColorSpace is not RGBColorspace as expected" << endl;
      }

    // Test set/get
    image.quantizeColorSpace(YIQColorspace);
    if ( image.quantizeColorSpace() != YIQColorspace )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", quantizeColorSpace set/get failed" << endl;
      }
    image.quantizeColorSpace(RGBColorspace);

    //
    // quantizeDither
    //
    // Test default
    if ( image.quantizeDither() == false )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", quantizeDither is not false as expected" << endl;
      }

    // Test set/get
    image.quantizeDither(false);
    if ( image.quantizeDither() != false )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", quantizeDither get/set failed" << endl;
      }
    image.quantizeDither(true);

    //
    // quantizeTreeDepth
    //
    if ( image.quantizeTreeDepth() != 0 )
      {
	++failures;
	cout << "Line: " << __LINE__ << ", quantizeTreeDepth default is "
	     << image.quantizeTreeDepth()
	     << " rather than zero as expected" << endl;
      }

    image.quantizeTreeDepth(7);
    if ( image.quantizeTreeDepth() != 7 )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", quantizeTreeDepth set/get failed" << endl;
      }
    image.quantizeTreeDepth(8);

    //
    // renderingIntent
    //
    if ( image.renderingIntent() != UndefinedIntent )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", renderingIntent default is not UndefinedIntent as expected"
             << endl;
      }

    image.renderingIntent(PerceptualIntent);
    if ( image.renderingIntent() != PerceptualIntent )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", renderingIntent set/get failed" << endl;
      }
    image.renderingIntent(UndefinedIntent);

    //
    // resolutionUnits
    //
    if ( image.resolutionUnits() != UndefinedResolution )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", resolutionUnits default is not UndefinedResolution as expected"
             << endl;
      }

    image.resolutionUnits(PixelsPerCentimeterResolution);
    if ( image.resolutionUnits() != PixelsPerCentimeterResolution )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", resolutionUnits set/get failed" << endl;
      }
    image.resolutionUnits(UndefinedResolution);

    //
    // rows
    //
    if ( image.rows() != rows )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", rows is canvas rows as expected" << endl;
      }

    //
    // scene
    //
    if ( image.scene() != 0 )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", scene default is not zero as expected" << endl;
      }

    image.scene(5);
    if ( image.scene() != 5 )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", scene set/get failed" << endl;
      }
    image.scene(0);

    //
    // signature
    //

    if ( image.signature() != "c7ac1ef7b47015c6ea6c1fb1d736eba4f8c3fe81dbfe511fbce104cedfce7588" &&
	 image.signature() != "d9464cd4d0c02f25166909726d6548db51d25fa91bd3cff642813f8a464bcfc7" &&
	 image.signature() != "e073572dfa4ad28f2f8dd3c6d37dfb14585e60c94cfae910149e97eff2fd895f" &&
	 image.signature() != "e12b9781b3a5025628567a4eabf970d16d42560e1b86189caceb03ec358dd8e6" &&
	 image.signature() != "ed06047a79b5b298515538db3fb8186d79e94758ed07a9b411637ba3a79fb4a0" &&
   image.signature() != "7e5977b8bce5c40b858c84344803dae61feae0ef7a21739b2d068c9cdb72f95b")
      {
	++failures;
	cout << "Line: " << __LINE__ << ", signature ("
	     << image.signature()
	     << ") is incorrect" << endl;
	image.display();
      }

    //
    // size
    //
    if ( image.size() != geometry )
      {
	++failures;
	cout << "Line: " << __LINE__ << ", size ("
	     << string(image.size())
	     << ") is not equal to geometry ("
	     << string(geometry)
	     << ")"
	     << endl;
      }

    image.size("800x600");
    if ( image.size() != Geometry("800x600") )
      {
	++failures;
	cout << "Line: " << __LINE__ << ", size set/get failed" << endl;
      }
    image.size( geometry );

    //
    // subImage
    //
    if ( image.subImage() != 0 )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", subImage default is not zero as expected" << endl;
      }

    image.subImage(5);
    if ( image.subImage() != 5 )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", subImage set/get failed" << endl;
      }
    image.subImage(0);

    //
    // subRange
    //
    if ( image.subRange() != 0 )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", subRange default is not zero as expected" << endl;
      }

    image.subRange(5);
    if ( image.subRange() != 5 )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", subRange set/get failed" << endl;
      }
    image.subRange(0);

    //
    // tileName
    //
    if ( image.tileName().length() != 0 )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", tileName default is not empty string as expected" << endl;
      }
    
    image.tileName("How now brown cow?");
    if ( image.tileName() != "How now brown cow?" )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", tileName set/get failed" << endl;
      }

    image.tileName( string() );
    if ( image.tileName().length() != 0 )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", tileName failed to unset" << endl;
      }

    //
    // totalColors
    //
    if ( image.totalColors() != 2 )
      {
	++failures;
	cout << "Line: " << __LINE__ << ", totalColors is " << image.totalColors()
	     << " rather than 2 as expected" << endl;
      }

    //
    // type
    //
    image.type(PaletteType);
    if ( image.type() != PaletteType )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", type is not PaletteType as expected. Reported type "
             << (int) image.type() << endl;

      }

    //
    // verbose
    //
    if ( image.verbose() != false )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", verbose is not false as expected" << endl;
      }

    //
    // view
    //
    if ( image.view().length() != 0 )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", view default is not empty string as expected" << endl;
      }
    
    image.view("How now brown cow?");
    if ( image.view() != "How now brown cow?" )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", view set/get failed" << endl;
      }

    image.view( string() );
    if ( image.view().length() != 0 )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", view failed to unset" << endl;
      }

    //
    // x11Display
    //
    if ( image.x11Display().length() != 0 )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", x11Display default is not empty string as expected" << endl;
      }
    
    image.x11Display(":0.0");
    if ( image.x11Display() != ":0.0" )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", x11Display set/get failed" << endl;
      }

    image.x11Display( string() );
    if ( image.x11Display().length() != 0 )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", x11Display failed to unset" << endl;
      }

    //
    // xResolution
    //
    if ( image.xResolution() != 72 )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", xResolution default (" << image.xResolution()
             << ") is not zero as expected" << endl;
      }

    //
    // yResolution
    //
    if ( image.yResolution() != 72 )
      {
	++failures;
	cout << "Line: " << __LINE__
             << ", yResolution default (" << image.yResolution()
             << ") is not zero as expected" << endl;
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
