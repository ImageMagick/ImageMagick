#!/bin/bash
# Benchmark Metal vs OpenCL vs CPU
# Usage: ./bench_metal_vs_opencl.sh <operation>

MAGICK=./utilities/magick
SIZE=8000x8000
RUNS=3

bench_op() {
  local name="$1"; shift
  local op="$@"
  echo "=== $name ==="

  echo "  Metal:"
  for i in $(seq 1 $RUNS); do
    MAGICK_OCL_DEVICE=OFF time $MAGICK -size $SIZE xc: +noise Random $op /dev/null 2>/dev/null
  done

  echo "  OpenCL:"
  for i in $(seq 1 $RUNS); do
    MAGICK_DISABLE_METAL=1 time $MAGICK -size $SIZE xc: +noise Random $op /dev/null 2>/dev/null
  done

  echo "  CPU:"
  for i in $(seq 1 $RUNS); do
    MAGICK_DISABLE_METAL=1 MAGICK_OCL_DEVICE=OFF time $MAGICK -size $SIZE xc: +noise Random $op /dev/null 2>/dev/null
  done
  echo ""
}

case "${1:-help}" in
  contrast)       bench_op "Contrast" -contrast ;;
  blur)           bench_op "Blur sigma=20" -blur 0x20 ;;
  grayscale)      bench_op "Grayscale" -grayscale Average ;;
  function)       bench_op "Function Sinusoid" -function Sinusoid 4,0,0.5,0.5 ;;
  motionblur)     bench_op "MotionBlur" -alpha on -motion-blur 0x20+45 ;;
  rotationalblur) bench_op "RotationalBlur" -rotational-blur 10 ;;
  unsharp)        bench_op "UnsharpMask" -unsharp 0x5+1+0.05 ;;
  despeckle)      bench_op "Despeckle" -alpha on -despeckle ;;
  localcontrast)  bench_op "LocalContrast" -alpha on -local-contrast 10x5 ;;
  equalize)       bench_op "Equalize" -alpha on -equalize ;;
  resize)         bench_op "Resize 50%" -resize 50% ;;
  wavelet)        bench_op "WaveletDenoise" -wavelet-denoise 5% ;;
  *)
    echo "Usage: $0 {contrast|blur|grayscale|function|motionblur|rotationalblur|unsharp|despeckle|localcontrast|equalize|resize|wavelet}"
    ;;
esac
