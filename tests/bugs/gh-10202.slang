//TEST_IGNORE_FILE:

// Regression test: constant folding >> 64 on uint64_t produces wrong value (UB in compiler)
// Issue: https://github.com/shader-slang/slang/issues/10202
// Expected: out[0] == 0.  Actual: out[0] == 0x8000000000000000 (unchanged).

RWStructuredBuffer<uint64_t> out;

[numthreads(1,1,1)]
void computeMain() {
    const uint64_t topBitSet = 0x8000000000000000ull;
    const uint64_t shifted = topBitSet >> 64;
    out[0] = shifted;
}
