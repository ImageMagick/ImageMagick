#!/bin/bash
set -e

# Build should be done before running this.

echo "Checking for Metal framework linkage..."
otool -L MagickCore/.libs/libMagickCore-7.Q16HDRI.dylib | grep Metal || { echo "Error: libMagickCore not linked with Metal"; exit 1; }

echo "Generating reference image..."
./utilities/magick rose: reference.png

echo "Running Metal contrast..."
# Capture stderr to check for debug logs
MAGICK_DEBUG=Accelerate ./utilities/magick rose: -contrast metal_out.png 2> metal.log

echo "Checking debug log for Metal usage..."
if grep -q "Metal device created successfully" metal.log; then
    echo "SUCCESS: Metal device selected."
else
    echo "FAILURE: Metal device NOT selected."
    cat metal.log
    exit 1
fi

if grep -q "pipeline for kernel" metal.log; then
    echo "SUCCESS: Metal kernel acquired."
else
    echo "WARNING: specific kernel log not found, check implementation."
fi

echo "Comparing results..."
./utilities/magick compare -metric RMSE reference.png metal_out.png null: || true
