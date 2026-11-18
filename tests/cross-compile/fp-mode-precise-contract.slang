// Regression test for #14567: on the C++ target, `-fp-mode precise` must leave a
// trace in the emitted source (a file-scope `#pragma STDC FP_CONTRACT OFF`) so
// the application's own C++ compiler does not contract `a*b+c` into an FMA,
// while default and fast modes emit no fp-contract pragma. The CUDA target
// must not inherit the pragma: clang's CUDA mode defines `__clang__`, so the
// source-language gate — not the compiler guard — is what keeps CUDA output
// unchanged.

// TEST:SIMPLE(filecheck=CHECK_PRECISE): -target cpp -entry computeMain -stage compute -fp-mode
// precise TEST:SIMPLE(filecheck=CHECK_DEFAULT): -target cpp -entry computeMain -stage compute
// TEST:SIMPLE(filecheck=CHECK_FAST): -target cpp -entry computeMain -stage compute -fp-mode fast
// TEST:SIMPLE(filecheck=CUDA_PRECISE): -target cuda -entry computeMain -stage compute -fp-mode
// precise

RWStructuredBuffer<float> gOut;

[shader("compute")][numthreads(1, 1, 1)] void computeMain(uint3 tid
                                                          : SV_DispatchThreadID)
{
    // Operands come from the buffer so the multiply-add cannot be constant-folded.
    float a = gOut[0], b = gOut[1], c = gOut[2];
    gOut[3] = a * b + c;
}

// The pragma must appear in the front matter, ahead of the prelude's includes:
// the STDC pragma is file-scope from its point onward, so emitting it before
// the prelude covers all module code. The guard lines are pinned too, so a
// regression that drops or mangles the compiler guard cannot pass unnoticed.
// CHECK_PRECISE: #if defined(__clang__)
// CHECK_PRECISE: #pragma STDC FP_CONTRACT OFF
// CHECK_PRECISE: #endif
// CHECK_PRECISE: #include

// CHECK_DEFAULT-NOT: FP_CONTRACT

// CHECK_FAST-NOT: FP_CONTRACT

// CUDA_PRECISE-NOT: FP_CONTRACT
