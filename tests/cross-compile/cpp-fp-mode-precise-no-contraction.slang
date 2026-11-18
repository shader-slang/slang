// Under `-fp-mode precise`, the C++ output tells the C++ compiler not to contract `a * b + c` into a
// fused multiply-add (Clang does it within an expression by default, GCC across expressions,
// wherever the target has FMA instructions). The pragmas come before the prelude, so the prelude's
// inline functions are compiled under them too. The default and fast modes leave contraction to
// the compiler.

//TEST:SIMPLE(filecheck=PRECISE): -target cpp -stage compute -entry computeMain -fp-mode precise
//TEST:SIMPLE(filecheck=DEFAULT): -target cpp -stage compute -entry computeMain
//TEST:SIMPLE(filecheck=FAST): -target cpp -stage compute -entry computeMain -fp-mode fast

// a = b = 1 + 2^-12 and c = -(1 + 2^-11): the exact product is 1 + 2^-11 + 2^-24, which rounds to
// 1 + 2^-11, so `a * b + c` is 0 as written, and 2^-24 (0x33800000) when it is fused. With
// clang-cl /O2 /arch:AVX2, the precise output of master gives 0x33800000.
RWStructuredBuffer<uint> io;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain()
{
    float a = asfloat(io[0]);
    float b = asfloat(io[1]);
    float c = asfloat(io[2]);
    io[3] = asuint(a * b + c);
}

// PRECISE: #if defined(__clang__)
// PRECISE-NEXT: #pragma clang fp contract(off)
// PRECISE-NEXT: #elif defined(__GNUC__)
// PRECISE-NEXT: #pragma GCC optimize("fp-contract=off")
// PRECISE-NEXT: #elif defined(_MSC_VER)
// PRECISE-NEXT: #pragma fp_contract(off)
// PRECISE-NEXT: #endif
// PRECISE: #include "{{.*}}slang-cpp-prelude.h"
// PRECISE: computeMain

// DEFAULT-NOT: fp contract
// DEFAULT-NOT: fp_contract
// DEFAULT: computeMain

// FAST-NOT: fp contract
// FAST-NOT: fp_contract
// FAST: computeMain
