// Regression test for #14567: on the C++ target, `-fp-mode precise` must leave a
// trace in the emitted source (a file-scope `#pragma STDC FP_CONTRACT OFF`) so
// the application's own C++ compiler does not contract `a*b+c` into an FMA,
// while default and fast modes emit no fp-contract pragma. The CUDA target
// must not inherit the pragma: nvcc defines `__GNUC__` but ignores STDC
// pragmas, and the CUDA fp-mode behavior is tracked separately (#14396).

//TEST:SIMPLE(filecheck=CHECK_PRECISE): -target cpp -entry computeMain -stage compute -fp-mode precise
//TEST:SIMPLE(filecheck=CHECK_DEFAULT): -target cpp -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK_FAST): -target cpp -entry computeMain -stage compute -fp-mode fast
//TEST:SIMPLE(filecheck=CUDA_PRECISE): -target cuda -entry computeMain -stage compute -fp-mode precise

RWStructuredBuffer<float> gOut;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // Operands come from the buffer so the multiply-add cannot be constant-folded.
    float a = gOut[0], b = gOut[1], c = gOut[2];
    gOut[3] = a * b + c;
}

// The pragma must appear in the front matter, ahead of the prelude's includes:
// the STDC pragma is file-scope from its point onward, so emitting it before
// the prelude covers all module code.
//CHECK_PRECISE: #pragma STDC FP_CONTRACT OFF
//CHECK_PRECISE: #include

//CHECK_DEFAULT-NOT: FP_CONTRACT

//CHECK_FAST-NOT: FP_CONTRACT

//CUDA_PRECISE-NOT: FP_CONTRACT
