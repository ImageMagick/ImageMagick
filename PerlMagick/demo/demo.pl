#!/usr/bin/perl
#
# Overall demo of the major PerlMagick methods.
#
use Image::Magick;

#
# Read model & smile image.
#
print "Read...\n";
$null=Image::Magick->new;
$null->Set(size=>'70x70');
$x=$null->ReadImage('NULL:black');
warn "$x" if "$x";

$model=Image::Magick->new();
$x=$model->ReadImage('model.gif');
warn "$x" if "$x";
$model->Label('Magick');
$model->Set(background=>'white');

$smile=Image::Magick->new;
$x=$smile->ReadImage('smile.gif');
warn "$x" if "$x";
$smile->Label('Smile');
$smile->Set(background=>'white');
#
# Create image stack.
#
print "Transform image...\n";
$images=Image::Magick->new();

print "Adaptive Blur...\n";
$example=$model->Clone();
$example->Label('Adaptive Blur');
$example->AdaptiveBlur('0x1');
push(@$images,$example);

print "Adaptive Resize...\n";
$example=$model->Clone();
$example->Label('Adaptive Resize');
$example->AdaptiveResize('60%');
push(@$images,$example);

print "Adaptive Sharpen...\n";
$example=$model->Clone();
$example->Label('Adaptive Sharpen');
$example->AdaptiveSharpen('0x1');
push(@$images,$example);

print "Adaptive Threshold...\n";
$example=$model->Clone();
$example->Label('Adaptive Threshold');
$example->AdaptiveThreshold('5x5+5%');
push(@$images,$example);

print "Add Noise...\n";
$example=$model->Clone();
$example->Label('Add Noise');
$example->AddNoise("Laplacian");
push(@$images,$example);

print "Annotate...\n";
$example=$model->Clone();
$example->Label('Annotate');
$example->Annotate(font=>'Generic.ttf',text=>'Magick',geometry=>'+0+20',
  fill=>'gold',gravity=>'North',pointsize=>14);
push(@$images,$example);

print "Auto-gamma...\n";
$example=$model->Clone();
$example->Label('Auto Gamma');
$example->AutoGamma();
push(@$images,$example);

print "Auto-level...\n";
$example=$model->Clone();
$example->Label('Auto Level');
$example->AutoLevel();
push(@$images,$example);

print "Auto-threshold...\n";
$example=$model->Clone();
$example->Label('Auto Threshold');
$example->AutoThreshold();
push(@$images,$example);

print "Blur...\n";
$example=$model->Clone();
$example->Label('Blur');
$example->Blur('0.0x1.0');
push(@$images,$example);

print "Border...\n";
$example=$model->Clone();
$example->Label('Border');
$example->Border(geometry=>'6x6',color=>'gold');
push(@$images,$example);

print "Channel...\n";
$example=$model->Clone();
$example->Label('Channel');
$example->Channel(channel=>'red');
push(@$images,$example);

print "Charcoal...\n";
$example=$model->Clone();
$example->Label('Charcoal');
$example->Charcoal('2x1');
push(@$images,$example);

print "ColorMatrix...\n";
$example=$model->Clone();
$example->Label('ColorMatrix');
$example->ColorMatrix([1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0.5, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 1]);
push(@$images,$example);

print "Colorspace...\n";
$example=$model->Clone();
$example->Label('Colorspace');
$example->Colorspace('Lab');
push(@$images,$example);

print "Composite...\n";
$example=$model->Clone();
$example->Label('Composite');
$example->Composite(image=>$smile,compose=>'over',geometry=>'+35+65');
$example->Clamp();
push(@$images,$example);

print "Contrast...\n";
$example=$model->Clone();
$example->Label('Contrast');
$example->Contrast();
push(@$images,$example);

print "Contrast Stretch...\n";
$example=$model->Clone();
$example->Label('Contrast Stretch');
$example->ContrastStretch('5%');
push(@$images,$example);

print "Convolve...\n";
$example=$model->Clone();
$example->Label('Convolve');
$example->Convolve([0.125, 0.125, 0.125, 0.125, 0.5, 0.125, 0.125, 0.125, 0.125]);
push(@$images,$example);

print "Crop...\n";
$example=$model->Clone();
$example->Label('Crop');
$example->Crop(geometry=>'80x80+25+50');
$example->Set(page=>'0x0+0+0');
push(@$images,$example);

print "Despeckle...\n";
$example=$model->Clone();
$example->Label('Despeckle');
$example->Despeckle();
push(@$images,$example);

print "Distort...\n";
$example=$model->Clone();
$example->Label('Distort');
$example->Distort(method=>'arc',points=>[60],'virtual-pixel'=>'white');
push(@$images,$example);

print "Draw...\n";
$example=$model->Clone();
$example->Label('Draw');
$example->Draw(fill=>'none',stroke=>'gold',primitive=>'circle',
  points=>'60,90 60,120',strokewidth=>2);
push(@$images,$example);

print "Detect Edges...\n";
$example=$model->Clone();
$example->Label('Detect Edges');
$example->Edge('2x0.5');
$example->Clamp();
push(@$images,$example);

print "Emboss...\n";
$example=$model->Clone();
$example->Label('Emboss');
$example->Emboss('0x1');
push(@$images,$example);

print "Encipher...\n";
$example=$model->Clone();
$example->Label('Encipher');
$example->Encipher('Magick');
push(@$images,$example);

print "Equalize...\n";
$example=$model->Clone();
$example->Label('Equalize');
$example->Equalize();
push(@$images,$example);

print "Explode...\n";
$example=$model->Clone();
$example->Label('Explode');
$example->Implode(-1);
push(@$images,$example);

print "Flip...\n";
$example=$model->Clone();
$example->Label('Flip');
$example->Flip();
push(@$images,$example);

print "Flop...\n";
$example=$model->Clone();
$example->Label('Flop');
$example->Flop();
push(@$images,$example);

print "Frame...\n";
$example=$model->Clone();
$example->Label('Frame');
$example->Frame('15x15+3+3');
push(@$images,$example);

print "Fx...\n";
$example=$model->Clone();
$example->Label('Fx');
push(@$images,$example->Fx(expression=>'0.5*u'));

print "Gamma...\n";
$example=$model->Clone();
$example->Label('Gamma');
$example->Gamma(1.6);
push(@$images,$example);

print "Gaussian Blur...\n";
$example=$model->Clone();
$example->Label('Gaussian Blur');
$example->GaussianBlur('0.0x1.5');
push(@$images,$example);

print "Gradient...\n";
$gradient=Image::Magick->new;
$gradient->Set(size=>'130x194');
$x=$gradient->ReadImage('gradient:#20a0ff-#ffff00');
warn "$x" if "$x";
$gradient->Label('Gradient');
push(@$images,$gradient);

print "Grayscale...\n";
$example=$model->Clone();
$example->Label('Grayscale');
$example->Set(type=>'grayscale');
push(@$images,$example);

print "Implode...\n";
$example=$model->Clone();
$example->Label('Implode');
$example->Implode(0.5);
push(@$images,$example);

print "Kuwahara...\n";
$example=$model->Clone();
$example->Label('Kuwahara');
$example->Kuwahara('0x1');
push(@$images,$example);

print "Level...\n";
$example=$model->Clone();
$example->Label('Level');
$example->Level('20%x');
$example->Clamp();
push(@$images,$example);

print "Linear stretch...\n";
$example=$model->Clone();
$example->Label('Linear Stretch');
$example->LinearStretch('5x5');
push(@$images,$example);

print "Median Filter...\n";
$example=$model->Clone();
$example->Label('Median Filter');
$example->MedianFilter('4x4');
push(@$images,$example);

print "Mode...\n";
$example=$model->Clone();
$example->Label('Mode');
$example->Mode('4x4');
push(@$images,$example);

print "Modulate...\n";
$example=$model->Clone();
$example->Label('Modulate');
$example->Modulate(brightness=>110,saturation=>110,hue=>110);
push(@$images,$example);
$example=$model->Clone();

print "Monochrome...\n";
$example=$model->Clone();
$example->Label('Monochrome');
$example->Quantize(colorspace=>'gray',colors=>2,dither=>'false');
push(@$images,$example);

print "Morphology...\n";
$example=$model->Clone();
$example->Label('Morphology');
$example->Morphology(method=>'Dilate',kernel=>'Diamond',iterations=>2);
push(@$images,$example);

print "Motion Blur...\n";
$example=$model->Clone();
$example->Label('Motion Blur');
$example->MotionBlur('0x13+10-10');
push(@$images,$example);

print "Negate...\n";
$example=$model->Clone();
$example->Label('Negate');
$example->Negate();
push(@$images,$example);

print "Normalize...\n";
$example=$model->Clone();
$example->Label('Normalize');
$example->Normalize();
push(@$images,$example);

print "Oil Paint...\n";
$example=$model->Clone();
$example->Label('Oil Paint');
$example->OilPaint('2x0.5');
push(@$images,$example);

print "Plasma...\n";
$plasma=Image::Magick->new;
$plasma->Set(size=>'130x194');
$x=$plasma->ReadImage('plasma:fractal');
warn "$x" if "$x";
$plasma->Label('Plasma');
push(@$images,$plasma);

print "Polaroid...\n";
$example=$model->Clone();
$example->Label('Polaroid');
$example->Polaroid(caption=>'Magick',angle=>-5.0,gravity=>'center');
push(@$images,$example);

print "Posterize...\n";
$example=$model->Clone();
$example->Label('Posterize');
$example->Posterize(5);
push(@$images,$example);

print "Quantize...\n";
$example=$model->Clone();
$example->Label('Quantize');
$example->Quantize();
push(@$images,$example);

print "Rotational Blur...\n";
$example=$model->Clone();
$example->Label('Rotational Blur');
$example->RotationalBlur(10);
push(@$images,$example);

print "Raise...\n";
$example=$model->Clone();
$example->Label('Raise');
$example->Raise('10x10');
push(@$images,$example);

print "Reduce Noise...\n";
$example=$model->Clone();
$example->Label('Reduce Noise');
$example->ReduceNoise('2x2');
push(@$images,$example);

print "Resize...\n";
$example=$model->Clone();
$example->Label('Resize');
$example->Resize('60%');
push(@$images,$example);

print "Roll...\n";
$example=$model->Clone();
$example->Label('Roll');
$example->Roll(geometry=>'+20+10');
push(@$images,$example);

print "Rotate...\n";
$example=$model->Clone();
$example->Label('Rotate');
$example->Rotate(45);
push(@$images,$example);

print "Sample...\n";
$example=$model->Clone();
$example->Label('Sample');
$example->Sample('60%');
push(@$images,$example);

print "Scale...\n";
$example=$model->Clone();
$example->Label('Scale');
$example->Scale('60%');
push(@$images,$example);

print "Segment...\n";
$example=$model->Clone();
$example->Label('Segment');
$example->Segment();
push(@$images,$example);

print "Shade...\n";
$example=$model->Clone();
$example->Label('Shade');
$example->Shade(geometry=>'30x30',gray=>'true');
push(@$images,$example);

print "Sharpen...\n";
$example=$model->Clone();
$example->Label('Sharpen');
$example->Sharpen('0.0x1.0');
$example->Clamp();
push(@$images,$example);

print "Shave...\n";
$example=$model->Clone();
$example->Label('Shave');
$example->Shave('10x10');
push(@$images,$example);

print "Shear...\n";
$example=$model->Clone();
$example->Label('Shear');
$example->Shear('-20x20');
push(@$images,$example);

print "Sketch...\n";
$example=$model->Clone();
$example->Label('Sketch');
$example->Set(colorspace=>'Gray');
$example->Sketch('0x20+120');
push(@$images,$example);

print "Sigmoidal Contrast...\n";
$example=$model->Clone();
$example->Label('Sigmoidal Contrast');
$example->SigmoidalContrast("3x50%");
push(@$images,$example);

print "Spread...\n";
$example=$model->Clone();
$example->Label('Spread');
$example->Spread();
push(@$images,$example);

print "Solarize...\n";
$example=$model->Clone();
$example->Label('Solarize');
$example->Solarize();
push(@$images,$example);

print "Swirl...\n";
$example=$model->Clone();
$example->Label('Swirl');
$example->Swirl(90);
push(@$images,$example);

print "Tint...\n";
$example=$model->Clone();
$example->Label('Tint');
$example->Tint('wheat');
push(@$images,$example);

print "Unsharp Mask...\n";
$example=$model->Clone();
$example->Label('Unsharp Mask');
$example->UnsharpMask('0.0x1.0');
$example->Clamp();
push(@$images,$example);

print "Vignette...\n";
$example=$model->Clone();
$example->Label('Vignette');
$example->Vignette('0x20');
push(@$images,$example);

print "Wave...\n";
$example=$model->Clone();
$example->Label('Wave');
$example->Wave('25x150');
push(@$images,$example);

print "WaveletDenoise...\n";
$example=$model->Clone();
$example->Label('Wavelet Denoise');
$example->WaveletDenoise('5%');
push(@$images,$example);

#
# Create image montage.
#
print "Montage...\n";
$montage=$images->Montage(font=>'Generic.ttf',geometry=>'140x160+8+4>',
  gravity=>'Center',tile=>'5x+10+200',compose=>'over',background=>'#ffffff',
  pointsize=>18,fill=>'#600',stroke=>'none',shadow=>'true');

$logo=Image::Magick->new();
$logo->Read('logo:');
$logo->Zoom('40%');
$montage->Composite(image=>$logo,gravity=>'North');

print "Write...\n";
$montage->Write('demo.pam');
print "Display...\n";
$montage->Write(magick=>'SHOW',title=>"PerlMagick Demo");
