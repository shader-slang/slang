#!/bin/bash
# Benchmark regression test for MMA pipeline changes.
# Compiles PTX, checks registers/spills, runs validation, and profiles with ncu.
#
# Usage:
#   ./benchmarks/run_regression.sh              # Full run (validation + ncu + timing)
#   ./benchmarks/run_regression.sh --quick      # Quick run (validation + timing only, no ncu)
#   ./benchmarks/run_regression.sh --ncu-only   # ncu profiling only (skip validation)
#
# Prerequisites:
#   - Release build with neural module: cmake --build --preset release
#   - Python venv with slangpy: source ~/pip_venv/bin/activate
#   - GPU clocks locked: sudo nvidia-smi --lock-gpu-clocks=2520,2520
#   - ncu_launcher built: benchmarks/ncu_launcher_new_mma
#   - Tin2 built: tin2/benchmarks/mlp_perf/build/bench_tin2_single_layer_*

set -e
cd "$(git rev-parse --show-toplevel)"

SLANGC=./build/Release/bin/slangc
INCLUDE_DIR=build/Release/lib/slang-standard-module-2026.3.1
NCU_LAUNCHER=./benchmarks/ncu_launcher_new_mma
BATCH=8192
WARPS=2
MODE="${1:-}"

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
NC='\033[0m'

echo "========================================================================"
echo "MMA Benchmark Regression Test"
echo "========================================================================"
echo "Batch: $BATCH, Warps: $WARPS"
echo ""

# ---- Step 1: Compile PTX and check registers/spills ----
echo "---- Compiling PTX ----"

declare -A PTX_FILES
declare -A ENTRIES
declare -A MODES
declare -A INPUT_SIZES
declare -A OUTPUT_SIZES

PTX_FILES[bias]=/tmp/regression_bias.ptx
ENTRIES[bias]=compute_bias_reduce
MODES[bias]=bias_reduce
INPUT_SIZES[bias]=256
OUTPUT_SIZES[bias]=64

PTX_FILES[outer]=/tmp/regression_outer.ptx
ENTRIES[outer]=compute_outer_product
MODES[outer]=outer_product
INPUT_SIZES[outer]=128
OUTPUT_SIZES[outer]=128

PTX_FILES[transpose]=/tmp/regression_transpose.ptx
ENTRIES[transpose]=compute_single_layer_transpose
MODES[transpose]=transpose
INPUT_SIZES[transpose]=128
OUTPUT_SIZES[transpose]=128

PTX_FILES[backward]=/tmp/regression_backward.ptx
ENTRIES[backward]=compute_backward
MODES[backward]=backward
INPUT_SIZES[backward]=128
OUTPUT_SIZES[backward]=128

declare -A SLANG_FILES
SLANG_FILES[bias]=benchmarks/benchmark_single_layer_bias_reduce.slang
SLANG_FILES[outer]=benchmarks/benchmark_single_layer_outer_product.slang
SLANG_FILES[transpose]=benchmarks/benchmark_single_layer_transpose.slang
SLANG_FILES[backward]=benchmarks/benchmark_single_layer_backward.slang

for kernel in bias outer transpose backward; do
    echo -n "  $kernel: "
    $SLANGC "${SLANG_FILES[$kernel]}" \
        -target ptx -entry "${ENTRIES[$kernel]}" -stage compute \
        -o "${PTX_FILES[$kernel]}" \
        -I "$INCLUDE_DIR" \
        -DINPUT_SIZE="${INPUT_SIZES[$kernel]}" \
        -DOUTPUT_SIZE="${OUTPUT_SIZES[$kernel]}" \
        -DSUBGROUP_COUNT=$WARPS 2>&1 | grep -i "error" && { echo -e "${RED}COMPILE FAILED${NC}"; exit 1; }

    head -c -1 "${PTX_FILES[$kernel]}" > "${PTX_FILES[$kernel]%.ptx}_fixed.ptx"
    PTXAS_OUT=$(/usr/local/cuda/bin/ptxas -v --gpu-name sm_89 "${PTX_FILES[$kernel]%.ptx}_fixed.ptx" -o /dev/null 2>&1)

    REGS=$(echo "$PTXAS_OUT" | grep -oP 'Used \K[0-9]+(?= registers)')
    SPILL_S=$(echo "$PTXAS_OUT" | grep -oP '(\K[0-9]+)(?= bytes spill stores)' || echo "0")
    SPILL_L=$(echo "$PTXAS_OUT" | grep -oP '(\K[0-9]+)(?= bytes spill loads)' || echo "0")
    echo "regs=$REGS, spill_stores=${SPILL_S}B, spill_loads=${SPILL_L}B"
done

echo ""

# ---- Step 2: Validation ----
if [ "$MODE" != "--ncu-only" ]; then
    echo "---- Validation ----"

    echo -n "  bias reduce: "
    BIAS_VAL=$(source ~/pip_venv/bin/activate && python3 benchmarks/benchmark_single_layer_bias_reduce.py --batch-size $BATCH --warps $WARPS --device cuda --all 2>&1)
    BIAS_PASS=$(echo "$BIAS_VAL" | grep -c "PASS" || true)
    BIAS_FAIL=$(echo "$BIAS_VAL" | grep -c "FAIL" || true)
    if [ "$BIAS_FAIL" -gt 0 ]; then
        echo -e "${RED}FAIL ($BIAS_FAIL failures)${NC}"
        echo "$BIAS_VAL" | grep -A 20 "FAIL"
    else
        echo -e "${GREEN}PASS ($BIAS_PASS/5)${NC}"
    fi

    echo -n "  outer product: "
    OPA_VAL=$(source ~/pip_venv/bin/activate && python3 benchmarks/benchmark_single_layer_outer_product.py --all --warps $WARPS --batch-size $BATCH --val 2>&1)
    OPA_PASS=$(echo "$OPA_VAL" | grep -c "PASS" || true)
    OPA_FAIL=$(echo "$OPA_VAL" | grep -c "FAIL" || true)
    if [ "$OPA_FAIL" -gt 0 ]; then
        echo -e "${RED}FAIL ($OPA_FAIL failures)${NC}"
        echo "$OPA_VAL" | grep -A 5 "FAIL"
    else
        echo -e "${GREEN}PASS ($OPA_PASS/5)${NC}"
    fi

    echo -n "  transpose: "
    TRANS_VAL=$(source ~/pip_venv/bin/activate && python3 benchmarks/benchmark_single_layer_transpose.py --all --warps $WARPS --batch-size $BATCH --val 2>&1)
    TRANS_PASS=$(echo "$TRANS_VAL" | grep -c "PASS" || true)
    TRANS_FAIL=$(echo "$TRANS_VAL" | grep -c "FAIL" || true)
    if [ "$TRANS_FAIL" -gt 0 ]; then
        echo -e "${RED}FAIL ($TRANS_FAIL failures)${NC}"
        echo "$TRANS_VAL" | grep -A 5 "FAIL"
    else
        echo -e "${GREEN}PASS ($TRANS_PASS/5)${NC}"
    fi

    echo ""
fi

# ---- Step 3: ncu profiling ----
if [ "$MODE" != "--quick" ]; then
    echo "---- ncu profiling (locked clocks) ----"

    CLOCKS=$(nvidia-smi --query-gpu=clocks.gr --format=csv,noheader 2>/dev/null | tr -d ' ')
    if [ "$CLOCKS" != "2520MHz" ]; then
        echo -e "${YELLOW}WARNING: GPU clocks not locked at 2520MHz (current: $CLOCKS)${NC}"
        echo "  Run: sudo nvidia-smi --lock-gpu-clocks=2520,2520"
    fi

    for kernel in bias outer transpose backward; do
        echo -n "  $kernel: "
        sudo env PATH=$PATH /usr/local/cuda/bin/ncu --set full --launch-skip 100 --launch-count 1 \
            -f -o /tmp/ncu_regression_${kernel} \
            $NCU_LAUNCHER "${PTX_FILES[$kernel]}" \
            --input-size "${INPUT_SIZES[$kernel]}" --output-size "${OUTPUT_SIZES[$kernel]}" \
            --batch-size $BATCH --warps $WARPS --mode "${MODES[$kernel]}" >/dev/null 2>&1

        METRICS=$(/usr/local/cuda/bin/ncu --import /tmp/ncu_regression_${kernel}.ncu-rep --print-summary per-kernel 2>&1)
        DURATION=$(echo "$METRICS" | grep "Duration" | awk '{print $NF}')
        REGS=$(echo "$METRICS" | grep "Registers Per Thread" | awk '{print $NF}')
        SPILL=$(echo "$METRICS" | grep "Local Memory Spilling Requests" | head -1 | awk '{print $NF}')
        WCPI=$(echo "$METRICS" | grep "Warp Cycles Per Issued" | awk '{print $NF}')
        echo "duration=${DURATION}us, regs=${REGS}, spill_req=${SPILL}, warp_cycles=${WCPI}"
    done

    echo ""
fi

# ---- Step 4: Timing comparison (ncu launcher, locked clocks) ----
echo "---- Timing comparison (3 runs each, locked clocks) ----"
echo ""
printf "%-12s %-12s %-12s %-12s\n" "Kernel" "Slang best" "Tin2 best" "Gap"
printf "%-12s %-12s %-12s %-12s\n" "--------" "----------" "---------" "---"

for kernel in bias outer transpose backward; do
    SLANG_BEST=999
    for run in 1 2 3; do
        T=$($NCU_LAUNCHER "${PTX_FILES[$kernel]}" \
            --input-size "${INPUT_SIZES[$kernel]}" --output-size "${OUTPUT_SIZES[$kernel]}" \
            --batch-size $BATCH --warps $WARPS --mode "${MODES[$kernel]}" 2>&1 | \
            grep "Avg time" | sed 's/.*: \([0-9.]*\) ms/\1/')
        if [ -n "$T" ] && (( $(echo "$T < $SLANG_BEST" | bc -l) )); then SLANG_BEST=$T; fi
    done

    TIN2_BEST=999
    case $kernel in
        bias) TIN2_CMD="./tin2/benchmarks/mlp_perf/build/bench_tin2_single_layer_bias_reduce --batch-size $BATCH --warps $WARPS --size large" ;;
        outer) TIN2_CMD="./tin2/benchmarks/mlp_perf/build/bench_tin2_single_layer_outer_product --batch-size $BATCH --warps $WARPS --size xlarge" ;;
        transpose) TIN2_CMD="./tin2/benchmarks/mlp_perf/build/bench_tin2_single_layer_transpose --batch-size $BATCH --warps $WARPS --size xlarge" ;;
        backward) TIN2_CMD="./tin2/benchmarks/mlp_perf/build/bench_tin2_single_layer_backward --batch-size $BATCH" ;;
    esac

    for run in 1 2 3; do
        T=$($TIN2_CMD 2>&1 | grep "Avg time" | tail -1 | sed 's/.*: \([0-9.]*\) ms/\1/')
        if [ -n "$T" ] && (( $(echo "$T < $TIN2_BEST" | bc -l) )); then TIN2_BEST=$T; fi
    done

    if (( $(echo "$TIN2_BEST > 0 && $TIN2_BEST < 999" | bc -l) )); then
        GAP=$(echo "($SLANG_BEST / $TIN2_BEST - 1) * 100" | bc -l)
        printf "%-12s %-12s %-12s %+.1f%%\n" "$kernel" "${SLANG_BEST} ms" "${TIN2_BEST} ms" "$GAP"
    else
        printf "%-12s %-12s %-12s %s\n" "$kernel" "${SLANG_BEST} ms" "N/A" "N/A"
    fi
done

echo ""
echo "========================================================================"
echo "Done."
