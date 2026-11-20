//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK,exit-code=0):

RWStructuredBuffer<uint64_t> out;
[numthreads(1,1,1)]
void computeMain()
{
    const uint64_t topBitSet = 0x8000000000000000ull;
    const uint64_t shifted = topBitSet >> 64;
//CHECK:                               ^^ target-defined value
//CHECK:                               ^^ the value of expression 9223372036854775808ULL >> 64 is target-defined.
    out[0] = shifted;
}
