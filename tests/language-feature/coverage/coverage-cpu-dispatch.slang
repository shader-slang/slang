// Regression test that the CPU backend packs the IR-synthesized
// `__slang_coverage` buffer into the standard `GlobalParams` struct
// alongside user-declared globals, so the kernel keeps the standard
// 3-argument ABI (varyingInput, entryPointParams, globalParams).
//
// The IR coverage pass extends the program-scope var layout so
// `collectGlobalUniformParameters` packs the synthesized buffer
// alongside user globals. If that extension regresses, the buffer
// escapes GlobalParams and reappears as a fourth kernel parameter,
// breaking host integrations that rely on the 3-arg ABI.

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
// globalParams). If global-uniform packing regresses and the
// coverage buffer escapes GlobalParams, a fourth typed param would
// appear here.
//CHECK: void _computeMain(void*{{[^,]*}}, void*{{[^,]*}}, void*{{[^)]*}})

// Counter increments happen against the coverage buffer accessed via
// globalParams — confirms the IR pass found and rewrote correctly.
//CHECK: _slang_atomic_add_u32{{.*}}_slang_coverage
