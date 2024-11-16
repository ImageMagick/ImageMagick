// This may look like C code, but it is really -*- C++ -*-
//
// Copyright Bob Friesenhahn, 2001, 2002, 2003
//
// Copyright @ 2013 ImageMagick Studio LLC, a non-profit organization
// dedicated to making software imaging solutions freely available.
//
// Resize image using specified resize algorithm with Magick++ API
//
// Usage: zoom [-density resolution] [-filter algorithm] [-geometry geometry]
//             [-resample resolution] input_file output_file
//

#include <Magick++.h>
#include <cstdlib>
#include <iostream>
#include <string>
using namespace std; 
using namespace Magick;

static void Usage ( char **argv )
{
  cout << "Usage: " << argv[0]
       << " [-density resolution] [-filter algorithm] [-geometry geometry]"
       << " [-resample resolution] input_file output_file" << endl
       << "   algorithm - bessel blackman box catrom cubic gaussian hamming hanning" << endl
       << "     hermite lanczos mitchell point quadratic sample scale sinc triangle" << endl;
  exit(1);
}

static void ParseError (int position, char **argv)
{
  cout << "Argument \"" <<  argv[position] << "\" at position" << position
       << "incorrect" << endl;
  Usage(argv);
}

int main(int argc,char **argv) 
{
  // Initialize ImageMagick install location for Windows
  MagickPlusPlusGenesis genesis(*argv);

  if ( argc < 2 )
    Usage(argv);

  enum ResizeAlgorithm
  {
    Zoom,
    Scale,
    Sample
  };

  {
    Geometry geometry;
    Magick::FilterType filter(LanczosFilter);
    Point density;
    Point resample;
    ResizeAlgorithm resize_algorithm=Zoom;

    int argv_index=1;
    while ((argv_index < argc - 2) && (*argv[argv_index] == '-'))
      {
        std::string command(argv[argv_index]);
        if (command.compare("-density") == 0)
          {
            argv_index++;
            try {
              density=Geometry(argv[argv_index]);
            }
            catch( exception &/* error_ */)
              {
                ParseError(argv_index,argv);
              }
            argv_index++;
            continue;
          }
        else if (command.compare("-filter") == 0)
          {
            argv_index++;
            std::string algorithm(argv[argv_index]);
            if (algorithm.compare("point") == 0)
              filter=PointFilter;
            else if (algorithm.compare("box") == 0)
              filter=BoxFilter;
            else if (algorithm.compare("triangle") == 0)
              filter=TriangleFilter;
            else if (algorithm.compare("hermite") == 0)
              filter=HermiteFilter;
            else if (algorithm.compare("hanning") == 0)
              filter=HanningFilter;
            else if (algorithm.compare("hamming") == 0)
              filter=HammingFilter;
            else if (algorithm.compare("blackman") == 0)
              filter=BlackmanFilter;
            else if (algorithm.compare("gaussian") == 0)
              filter=GaussianFilter;
            else if (algorithm.compare("quadratic") == 0)
              filter=QuadraticFilter;
            else if (algorithm.compare("cubic") == 0)
              filter=CubicFilter;
            else if (algorithm.compare("catrom") == 0)
              filter=CatromFilter;
            else if (algorithm.compare("mitchell") == 0)
              filter=MitchellFilter;
            else if (algorithm.compare("lanczos") == 0)
              filter=LanczosFilter;
            else if (algorithm.compare("bessel") == 0)
              filter=BesselFilter;
            else if (algorithm.compare("sinc") == 0)
              filter=SincFilter;
            else if (algorithm.compare("sample") == 0)
              resize_algorithm=Sample;
            else if (algorithm.compare("scale") == 0)
              resize_algorithm=Scale;
            else
              ParseError(argv_index,argv);
            argv_index++;
            continue;
          }
        else if (command.compare("-geometry") == 0)
          {
            argv_index++;
            try {
              geometry=Geometry(argv[argv_index]);
            }
            catch( exception &/* error_ */)
              {
                ParseError(argv_index,argv);
              }
            argv_index++;
            continue;
          }
        else if (command.compare("-resample") == 0)
          {
            argv_index++;
            try {
              resample=Geometry(argv[argv_index]);
            }
            catch( exception &/* error_ */)
              {
                ParseError(argv_index,argv);
              }
            argv_index++;
            continue;
          }
        ParseError(argv_index,argv);
      }

    if (argv_index>argc-1)
      ParseError(argv_index,argv);
    std::string input_file(argv[argv_index]);
    argv_index++;
    if (argv_index>argc)
      ParseError(argv_index,argv);
    std::string output_file(argv[argv_index]);

    try {
      Image image(input_file);
      if (density.isValid())
        image.density(density);
      density=image.density();

      if (resample.isValid())
        {
          geometry =
            Geometry(static_cast<size_t>
                     (image.columns()*((double)resample.x()/density.x())+0.5),
                     static_cast<size_t>
                     (image.rows()*((double)resample.y()/density.y())+0.5));
          image.density(resample);
        }
      switch (resize_algorithm)
        {
        case Sample:
          image.sample(geometry);
          break;
        case Scale:
          image.scale(geometry);
          break;
        case Zoom:
          image.filterType(filter);
          image.zoom(geometry);
          break;
        }
      image.write(output_file);
    }
    catch( exception &error_ )
      {
        cout << "Caught exception: " << error_.what() << endl;
        return 1;
      }
  }

  return 0;
}
