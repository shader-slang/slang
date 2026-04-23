// Regression test that the CPU backend treats the AST-synthesized
// `__slang_coverage` buffer as a first-class reflection-visible
// resource — packed into the standard `GlobalParams` struct alongside
// user-declared globals, accessed via the standard 3-argument kernel
// ABI (varyingInput, entryPointParams, globalParams).
//
// If AST-time synthesis regresses (e.g. back to IR-time synthesis),
// the buffer escapes GlobalParams and re-appears as a fourth kernel
// parameter. Downstream C++ compilation rejects the wrappers, and
// slang-rhi's reflection-driven binding can't find the buffer to
// bind at dispatch time.

//TEST:SIMPLE(filecheck=CHECK):-target cpp -stage compute -entry computeMain -trace-coverage

//TEST_INPUT: set outputBuffer = out ubuffer(data=[0], stride=4)
RWStructuredBuffer<uint> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[0] = 42u;
}

// The synthesized buffer appears as a field of GlobalParams, not as a
// separate global or kernel parameter.
//CHECK: struct GlobalParams
//CHECK: RWStructuredBuffer{{.*}}_slang_coverage

// Kernel keeps the standard 3-arg ABI (varying, entryPointParams,
// globalParams). If AST-synth regressed and the buffer escaped
// GlobalParams, a fourth typed param would appear here.
//CHECK: void _computeMain(void*{{[^,]*}}, void*{{[^,]*}}, void*{{[^)]*}})

// Counter increments happen against the coverage buffer accessed via
// globalParams — confirms the IR pass found and rewrote correctly.
//CHECK: _slang_atomic_add_u32{{.*}}_slang_coverage
