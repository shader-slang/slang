// The explicit NVVM method reaches its dedicated route and fails honestly until Slice 6 adds the
// Slang-IR-to-NVVM producer. This diagnostic is independent of whether libNVVM is installed.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -o nvvm-routing-not-implemented.ptx

[numthreads(1, 1, 1)]
void computeMain()
{}

//CHECK: E52014
