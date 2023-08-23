// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 1999, 2000, 2001, 2002, 2003
//
// Copyright @ 2013 ImageMagick Studio LLC, a non-profit organization
// dedicated to making software imaging solutions freely available.
//
// Simple demo program for Magick++
//
// Concept and algorithms lifted from PerlMagick demo script written
// by Cristy.
//
// Max run-time size 60MB (as compared with 95MB for PerlMagick) under SPARC Solaris
//

#include <Magick++.h>
#include <string>
#include <iostream>
#include <list>

using namespace std;

using namespace Magick;

#if defined(MAGICKCORE_FREETYPE_DELEGATE)
  #define MakeLabel(image, text) image.label( (text) )
#else
  #define MakeLabel(image, text)
#endif

int main( int /*argc*/, char ** argv)
{

  // Initialize ImageMagick install location for Windows
  InitializeMagick(*argv);

  const char *const p = getenv("MAGICK_FONT");
  const string MAGICK_FONT(p ? p : "");
  
  try {
    
    string srcdir("");
    if(getenv("SRCDIR") != 0)
      srcdir = getenv("SRCDIR");
    
    list<Image> montage;

    {
      //
      // Read model & smile image.
      //
      cout << "Read images ..." << endl;

      Image model( srcdir + "model.miff" );
      MakeLabel(model, "Magick++");
      model.borderColor( "black" );
      model.backgroundColor( "black" );
    
      Image smile( srcdir + "smile.miff" );
      MakeLabel(smile, "Smile");
      smile.borderColor( "black" );
    
      //
      // Create image stack.
      //
      cout << "Creating thumbnails..." << endl;
    
      // Construct initial list containing seven copies of a null image
      Image null;
      null.size( Geometry(70,70) );
      null.read( "NULL:black" );
      list<Image> images( 7, null );
    
      Image example = model;
    
      // Each of the following follow the pattern
      //  1. obtain reference to (own copy of) image
      //  2. apply label to image
      //  3. apply operation to image
      //  4. append image to container

      cout << "  add noise ..." << endl;
      MakeLabel(example, "Add Noise");
      example.addNoise( LaplacianNoise );
      images.push_back( example );

      cout << "  add noise (blue) ..." << endl;
      MakeLabel(example, "Add Noise\n(Blue Channel)");
      example.addNoiseChannel( BlueChannel, PoissonNoise );
      images.push_back( example );

#if defined(MAGICKCORE_FREETYPE_DELEGATE)
      cout << "  annotate ..." << endl;
      example = model;
      MakeLabel(example, "Annotate");
      example.density( "72x72" );
      example.fontPointsize( 18 );
      example.font(MAGICK_FONT);
      example.strokeColor( Color() );
      example.fillColor( "gold" );
      example.annotate( "Magick++", "+0+20", NorthGravity );
      images.push_back( example );
#endif

      cout << "  blur ..." << endl;
      example = model;
      MakeLabel(example, "Blur");
      example.blur( 0, 1.5 );
      images.push_back( example );

      cout << "  blur red channel ..." << endl;
      example = model;
      MakeLabel(example, "Blur Channel\n(Red Channel)");
      example.blurChannel( RedChannel, 0, 3.0 );
      images.push_back( example );

      cout << "  border ..." << endl;
      example = model;
      MakeLabel(example, "Border");
      example.borderColor( "gold" );
      example.border( Geometry(6,6) );
      images.push_back( example );

      cout << "  channel ..." << endl;
      example = model;
      MakeLabel(example, "Channel\n(Red Channel)");
      example.channel( RedChannel );
      images.push_back( example );

      cout << "  charcoal ..." << endl;
      example = model;
      MakeLabel(example, "Charcoal");
      example.charcoal( );
      images.push_back( example );

      cout << "  composite ..." << endl;
      example = model;
      MakeLabel(example, "Composite");
      example.composite( smile, "+35+65", OverCompositeOp);
      images.push_back( example );

      cout << "  contrast ..." << endl;
      example = model;
      MakeLabel(example, "Contrast");
      example.contrast( false );
      images.push_back( example );

      cout << "  convolve ..." << endl;
      example = model;
      MakeLabel(example, "Convolve");
      {
        // 3x3 matrix
        const double kernel[] = { 1, 1, 1, 1, 4, 1, 1, 1, 1 };
        example.convolve( 3, kernel );
      }
      images.push_back( example );

      cout << "  crop ..." << endl;
      example = model;
      MakeLabel(example, "Crop");
      example.crop( "80x80+25+50" );
      images.push_back( example );

      cout << "  despeckle ..." << endl;
      example = model;
      MakeLabel(example, "Despeckle");
      example.despeckle( );
      images.push_back( example );

      cout << "  draw ..." << endl;
      example = model;
      MakeLabel(example, "Draw");
      example.fillColor(Color());
      example.strokeColor( "gold" );
      example.strokeWidth( 2 );
      example.draw( DrawableCircle( 60,90, 60,120 ) );
      images.push_back( example );

      cout << "  edge ..." << endl;
      example = model;
      MakeLabel(example, "Detect Edges");
      example.edge( );
      images.push_back( example );

      cout << "  emboss ..." << endl;
      example = model;
      MakeLabel(example, "Emboss");
      example.emboss( );
      images.push_back( example );

      cout << "  equalize ..." << endl;
      example = model;
      MakeLabel(example, "Equalize");
      example.equalize( );
      images.push_back( example );
    
      cout << "  explode ..." << endl;
      example = model;
      MakeLabel(example, "Explode");
      example.backgroundColor( "#000000FF" );
      example.implode( -1 );
      images.push_back( example );

      cout << "  flip ..." << endl;
      example = model;
      MakeLabel(example, "Flip");
      example.flip( );
      images.push_back( example );

      cout << "  flop ..." << endl;
      example = model;
      MakeLabel(example, "Flop");
      example.flop();
      images.push_back( example );

      cout << "  frame ..." << endl;
      example = model;
      MakeLabel(example, "Frame");
      example.frame( );
      images.push_back( example );

      cout << "  gamma ..." << endl;
      example = model;
      MakeLabel(example, "Gamma");
      example.gamma( 1.6 );
      images.push_back( example );

      cout << "  gaussian blur ..." << endl;
      example = model;
      MakeLabel(example, "Gaussian Blur");
      example.gaussianBlur( 0.0, 1.5 );
      images.push_back( example );

      cout << "  gaussian blur channel ..." << endl;
      example = model;
      MakeLabel(example, "Gaussian Blur\n(Green Channel)");
      example.gaussianBlurChannel( GreenChannel, 0.0, 1.5 );
      images.push_back( example );
    
      cout << "  gradient ..." << endl;
      Image gradient;
      gradient.size( "130x194" );
      gradient.read( "gradient:#20a0ff-#ffff00" );
      MakeLabel(gradient, "Gradient");
      images.push_back( gradient );
    
      cout << "  grayscale ..." << endl;
      example = model;
      MakeLabel(example, "Grayscale");
      example.quantizeColorSpace( GRAYColorspace );
      example.quantize( );
      images.push_back( example );
    
      cout << "  implode ..." << endl;
      example = model;
      MakeLabel(example, "Implode");
      example.implode( 0.5 );
      images.push_back( example );

      cout << "  level ..." << endl;
      example = model;
      MakeLabel(example, "Level");
      example.level( 0.20*QuantumRange, 0.90*QuantumRange, 1.20 );
      images.push_back( example );

      cout << "  level red channel ..." << endl;
      example = model;
      MakeLabel(example, "Level Channel\n(Red Channel)");
      example.levelChannel( RedChannel, 0.20*QuantumRange, 0.90*QuantumRange, 1.20 );
      images.push_back( example );

      cout << "  median filter ..." << endl;
      example = model;
      MakeLabel(example, "Median Filter");
      example.medianFilter( );
      images.push_back( example );

      cout << "  modulate ..." << endl;
      example = model;
      MakeLabel(example, "Modulate");
      example.modulate( 110, 110, 110 );
      images.push_back( example );

      cout << "  monochrome ..." << endl;
      example = model;
      MakeLabel(example, "Monochrome");
      example.quantizeColorSpace( GRAYColorspace );
      example.quantizeColors( 2 );
      example.quantizeDither( false );
      example.quantize( );
      images.push_back( example );

      cout << "  motion blur ..." << endl;
      example = model;
      MakeLabel(example, "Motion Blur");
      example.motionBlur( 0.0, 7.0,45 );
      images.push_back( example );
    
      cout << "  negate ..." << endl;
      example = model;
      MakeLabel(example, "Negate");
      example.negate( );
      images.push_back( example );
    
      cout << "  normalize ..." << endl;
      example = model;
      MakeLabel(example, "Normalize");
      example.normalize( );
      images.push_back( example );
    
      cout << "  oil paint ..." << endl;
      example = model;
      MakeLabel(example, "Oil Paint");
      example.oilPaint( );
      images.push_back( example );

      cout << "  ordered dither 2x2 ..." << endl;
      example = model;
      MakeLabel(example, "Ordered Dither\n(2x2)");
      example.randomThreshold(2,2);
      images.push_back( example );

      cout << "  ordered dither 3x3..." << endl;
      example = model;
      MakeLabel(example, "Ordered Dither\n(3x3)");
      example.randomThreshold(3,3);
      images.push_back( example );

      cout << "  ordered dither 4x4..." << endl;
      example = model;
      MakeLabel(example, "Ordered Dither\n(4x4)");
      example.randomThreshold(4,4);
      images.push_back( example );
    
      cout << "  ordered dither red 4x4..." << endl;
      example = model;
      MakeLabel(example, "Ordered Dither\n(Red 4x4)");
      example.randomThresholdChannel(RedChannel,4,4);
      images.push_back( example );

      cout << "  plasma ..." << endl;
      Image plasma;
      plasma.size( "130x194" );
      plasma.read( "plasma:fractal" );
      MakeLabel(plasma, "Plasma");
      images.push_back( plasma );
    
      cout << "  quantize ..." << endl;
      example = model;
      MakeLabel(example, "Quantize");
      example.quantize( );
      images.push_back( example );

      cout << "  quantum operator ..." << endl;
      example = model;
      MakeLabel(example, "Quantum Operator\nRed * 0.4");
      example.evaluate( RedChannel,MultiplyEvaluateOperator,0.40 );
      images.push_back( example );

      cout << "  raise ..." << endl;
      example = model;
      MakeLabel(example, "Raise");
      example.raise( );
      images.push_back( example );
    
      cout << "  reduce noise ..." << endl;
      example = model;
      MakeLabel(example, "Reduce Noise");
      example.reduceNoise( 1.0 );
      images.push_back( example );
    
      cout << "  resize ..." << endl;
      example = model;
      MakeLabel(example, "Resize");
      example.zoom( "50%" );
      images.push_back( example );
    
      cout << "  roll ..." << endl;
      example = model;
      MakeLabel(example, "Roll");
      example.roll( "+20+10" );
      images.push_back( example );
    
      cout << "  rotate ..." << endl;
      example = model;
      MakeLabel(example, "Rotate");
      example.rotate( 45 );
      example.transparent( "black" );
      images.push_back( example );

      cout << "  scale ..." << endl;
      example = model;
      MakeLabel(example, "Scale");
      example.scale( "60%" );
      images.push_back( example );
    
      cout << "  segment ..." << endl;
      example = model;
      MakeLabel(example, "Segment");
      example.segment( 0.5, 0.25 );
      images.push_back( example );
    
      cout << "  shade ..." << endl;
      example = model;
      MakeLabel(example, "Shade");
      example.shade( 30, 30, false );
      images.push_back( example );
    
      cout << "  sharpen ..." << endl;
      example = model;
      MakeLabel(example, "Sharpen");
      example.sharpen( 0.0, 1.0 );
      images.push_back( example );
    
      cout << "  shave ..." << endl;
      example = model;
      MakeLabel(example, "Shave");
      example.shave( Geometry( 10, 10) );
      images.push_back( example );
    
      cout << "  shear ..." << endl;
      example = model;
      MakeLabel(example, "Shear");
      example.shear( 45, 45 );
      example.transparent( "black" );
      images.push_back( example );
    
      cout << "  spread ..." << endl;
      example = model;
      MakeLabel(example, "Spread");
      example.spread( 3 );
      images.push_back( example );
    
      cout << "  solarize ..." << endl;
      example = model;
      MakeLabel(example, "Solarize");
      example.solarize( );
      images.push_back( example );
    
      cout << "  swirl ..." << endl;
      example = model;
      example.backgroundColor( "#000000FF" );
      MakeLabel(example, "Swirl");
      example.swirl( 90 );
      images.push_back( example );

      cout << "  threshold ..." << endl;
      example = model;
      MakeLabel(example, "Threshold");
      example.threshold( QuantumRange/2.0 );
      images.push_back( example );

      cout << "  threshold random ..." << endl;
      example = model;
      MakeLabel(example, "Random\nThreshold");
      example.randomThreshold( (0.3*QuantumRange),
        (0.85*QuantumRange) );
      images.push_back( example );
    
      cout << "  unsharp mask ..." << endl;
      example = model;
      MakeLabel(example, "Unsharp Mask");
      //           radius_, sigma_, amount_, threshold_
      example.unsharpmask( 0.0, 1.0, 1.0, 0.05);
      images.push_back( example );
    
      cout << "  wave ..." << endl;
      example = model;
      MakeLabel(example, "Wave");
      example.alpha( true );
      example.backgroundColor( "#000000FF" );
      example.wave( 25, 150 );
      images.push_back( example );
    
      //
      // Create image montage.
      //
      cout <<  "Montage images..." << endl;

      for_each( images.begin(), images.end(), strokeColorImage( Color("#600") ) );

      MontageFramed montageOpts;
      montageOpts.geometry( "130x194+10+5>" );
      montageOpts.gravity( CenterGravity );
      montageOpts.borderColor( "green" );
      montageOpts.borderWidth( 1 );
      montageOpts.tile( "7x4" );
      montageOpts.backgroundColor( "#ffffff" );
      montageOpts.pointSize( 18 );
      montageOpts.font(MAGICK_FONT);
      montageOpts.fillColor( "#600" );
      montageOpts.strokeColor( Color() );
      montageOpts.fileName( "Magick++ Demo" );
      montageImages( &montage, images.begin(), images.end(), montageOpts );
    }

    Image& montage_image = montage.front();
    {
      // Create logo image
      cout << "Adding logo image ..." << endl;
      Image logo( "logo:" );
      logo.zoom( "45%" );

      // Composite logo into montage image
      Geometry placement(0,0,((ssize_t) montage_image.columns()/2)-
        ((ssize_t) logo.columns()/2),0);
      montage_image.composite( logo, placement, OverCompositeOp );
    }

    for_each( montage.begin(), montage.end(), depthImage(8) );
    for_each( montage.begin(), montage.end(), alphaImage( false ) );
    for_each( montage.begin(), montage.end(), compressTypeImage( RLECompression) );

    cout << "Writing image \"demo_out.miff\" ..." << endl;
    writeImages(montage.begin(),montage.end(),"demo_out_%d.miff");

    // Uncomment following lines to display image to screen
    //    cout <<  "Display image..." << endl;
    //    montage_image.display();

  }
  catch( exception &error_ )
    {
      cout << "Caught exception: " << error_.what() << endl;
      return 1;
    }

  return 0;
}
