//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -profile lib_6_6
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry computeMain -stage compute -O0
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target wgsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target wgsl -entry computeMain -stage compute -O0

// `operator&` is not available on WGSL and HLSL, but that is checked for the code that the entry
// points reach. An exported function that no entry point calls can still take the address of a
// global variable as a `T*`. No pointer value is introduced there, so the function that returns
// the pointer is reported (#13632).

static uint counter = 0;

[noinline] uint* counterAddress()
//CHECK:         ^^^^^^^^^^^^^^ pointers to memory addresses are not supported on this target
//CHECK:         ^^^^^^^^^^^^^^ this uses a pointer to a memory address, which the current code generation target cannot represent
{
    return &counter;
}

export uint readCounter()
{
    return *counterAddress();
}

RWStructuredBuffer<uint> outputBuffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    outputBuffer[tid.x] = 1;
}
