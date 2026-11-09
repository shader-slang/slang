// Regression test for #12799: on CUDA/NVRTC, `-fp-mode precise` must disable FMA
// contraction (NVRTC contracts `a*b+c` by default, which can change the result),
// while default mode is free to contract.

//TEST:SIMPLE(filecheck=CHECK_PRECISE): -target ptx -entry computeMain -stage compute -fp-mode precise
//TEST:SIMPLE(filecheck=CHECK_DEFAULT): -target ptx -entry computeMain -stage compute

RWStructuredBuffer<float> gOut;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // Operands come from the buffer so the multiply-add cannot be constant-folded.
    float a = gOut[0], b = gOut[1], c = gOut[2];
    gOut[3] = a * b + c;
}

// The `-NOT`s before, between, and after the positive checks assert `fma.rn`
// appears nowhere in the precise output, while the `mul.rn`/`add.rn` checks
// ensure the arithmetic was emitted (not folded away, which would satisfy a
// bare `-NOT` vacuously).
//CHECK_PRECISE-NOT: fma.rn
//CHECK_PRECISE: mul.rn.f32
//CHECK_PRECISE-NOT: fma.rn
//CHECK_PRECISE: add.rn.f32
//CHECK_PRECISE-NOT: fma.rn

//CHECK_DEFAULT: fma.rn
