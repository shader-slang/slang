//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -stage compute -entry computeMain -g1 -debug-info-include-source

// `-debug-info-include-source` only affects SPIR-V output. When requested for a non-SPIR-V target
// it produces a warning (E00021) rather than silently doing nothing, mirroring how
// `-separate-debug-info` warns for unsupported targets.

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    outputBuffer[0] = 1.0f;
}

//CHECK: E00021
