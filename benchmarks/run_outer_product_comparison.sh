#!/bin/bash
# Outer Product Benchmark Comparison: Slang MMAHelperNew vs Tin2 OuterProductReducer
# Uses ncu_launcher for fair GPU-timestamp comparison (no framework overhead).

set -e

SLANGC=./build/Release/bin/slangc
NVCC=/usr/local/cuda/bin/nvcc
LAUNCHER=/tmp/ncu_launcher_new_mma
TIN2_BIN=/tmp/bench_tin2_outer
BATCH_SIZE=${1:-256}
WARPS=${2:-1}

echo "======================================================================"
echo "Outer Product Benchmark Comparison (ncu_launcher)"
echo "======================================================================"
echo "Batch size: $BATCH_SIZE, Warps: $WARPS"
echo ""

# Build ncu_launcher if needed
if [ ! -f "$LAUNCHER" ]; then
    echo "Building ncu_launcher..."
    $NVCC -o $LAUNCHER benchmarks/ncu_launcher_new_mma.cu -lcuda -arch=sm_89 2>/dev/null
fi

# Build Tin2 benchmark if needed
if [ ! -f "$TIN2_BIN" ]; then
    echo "Building Tin2 benchmark..."
    $NVCC -o $TIN2_BIN tin2/benchmarks/mlp_perf/bench_tin2_single_layer_outer_product.cu -I tin2/include -arch=sm_89 2>/dev/null
fi

declare -A CONFIGS
CONFIGS[tiny]="32 16"
CONFIGS[small]="64 16"
CONFIGS[medium]="128 32"
CONFIGS[large]="256 64"
CONFIGS[xlarge]="128 128"

ORDER=(tiny small medium large xlarge)

declare -A SLANG_TIMES
declare -A TIN2_TIMES

for size in "${ORDER[@]}"; do
    read INPUT OUTPUT <<< "${CONFIGS[$size]}"
    PTX_FILE="/tmp/outer_product_${size}.ptx"

    echo "----------------------------------------------------------------------"
    echo "Config: ${size} (${OUTPUT}x${INPUT})"
    echo "----------------------------------------------------------------------"

    # Compile Slang PTX
    SC=${WARPS}
    $SLANGC benchmarks/benchmark_single_layer_outer_product.slang \
        -target ptx -o "$PTX_FILE" \
        -DINPUT_SIZE=$INPUT -DOUTPUT_SIZE=$OUTPUT -DSUBGROUP_COUNT=$SC \
        -entry compute_outer_product -experimental-feature 2>/dev/null

    # Fix trailing null byte for ptxas
    truncate -s -1 "$PTX_FILE" 2>/dev/null || true

    # Run Slang 3 times, take best
    BEST_SLANG=999
    for run in 1 2 3; do
        T=$($LAUNCHER "$PTX_FILE" --mode outer_product \
            --input-size $INPUT --output-size $OUTPUT \
            --batch-size $BATCH_SIZE --warps $WARPS 2>&1 | grep 'Avg time' | awk '{print $3}')
        echo "  Slang run $run: $T ms"
        if (( $(echo "$T < $BEST_SLANG" | bc -l) )); then
            BEST_SLANG=$T
        fi
    done
    SLANG_TIMES[$size]=$BEST_SLANG

    # Run Tin2 3 times, take best
    BEST_TIN2=999
    for run in 1 2 3; do
        T=$($TIN2_BIN --size $size --batch-size $BATCH_SIZE --warps $WARPS 2>&1 | grep 'Avg time' | awk '{print $3}')
        echo "  Tin2  run $run: $T ms"
        if (( $(echo "$T < $BEST_TIN2" | bc -l) )); then
            BEST_TIN2=$T
        fi
    done
    TIN2_TIMES[$size]=$BEST_TIN2
    echo ""
done

echo "======================================================================"
echo "Summary: Outer Product (best of 3 runs)"
echo "======================================================================"
printf "%-10s %-12s %-12s %-12s %s\n" "Size" "Dims" "Slang (ms)" "Tin2 (ms)" "Ratio"
echo "----------------------------------------------------------------------"

for size in "${ORDER[@]}"; do
    read INPUT OUTPUT <<< "${CONFIGS[$size]}"
    DIMS="${OUTPUT}x${INPUT}"
    S=${SLANG_TIMES[$size]}
    T=${TIN2_TIMES[$size]}
    RATIO=$(echo "scale=2; $S / $T" | bc -l)
    printf "%-10s %-12s %-12s %-12s %sx\n" "$size" "$DIMS" "$S" "$T" "$RATIO"
done
echo ""
