#!/bin/bash
set -e

# Benchmark Metal vs CPU

IMAGE="rose:"
# Use a larger image for meaningful benchmarking
LARGE_IMAGE="large_reference.miff"

echo "Generating large reference image (4096x4096)..."
./utilities/magick -size 4096x4096 xc:gray +noise Random $LARGE_IMAGE

echo "Verifying Metal usage..."
MAGICK_DEBUG=Accelerate ./utilities/magick $LARGE_IMAGE -contrast null: 2>&1 | grep "Selected Metal device" || echo "WARNING: Metal not detected during benchmark!"

echo "Running Benchmark..."
echo "--------------------------------------------------"
echo "| Configuration | Average Time (s) |"
echo "--------------------------------------------------"

run_benchmark() {
    local name=$1
    local env_vars=$2

    # Use 'time' to measure execution.
    # Validating generic time format output or just capturing stderr
    # Using a simple loop for stability

    # Warmup
    eval "$env_vars ./utilities/magick $LARGE_IMAGE -contrast null:"

    start=$(python3 -c 'import time; print(time.time())')
    for i in {1..5}; do
        eval "$env_vars ./utilities/magick $LARGE_IMAGE -contrast null:"
    done
    end=$(python3 -c 'import time; print(time.time())')

    runtime=$(echo "$end - $start" | bc)
    average=$(echo "$runtime / 5" | bc -l)

    printf "| %-13s | %0.4fs           |\n" "$name" "$average"
}

# Run Metal
# Ensure debug log is OFF to avoid I/O overhead affecting timing, unless we want to verify it's working
# But for timing we should probably trust it works based on previous verification.
run_benchmark "Metal" ""

# Run CPU
run_benchmark "CPU (-disable)" "MAGICK_DISABLE_METAL=1"

echo "--------------------------------------------------"
rm $LARGE_IMAGE
