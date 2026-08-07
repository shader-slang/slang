//TEST:COMPILE: tests/diagnostics/vector-dot-oversized-width-module.slang -o tests/diagnostics/vector-dot-oversized-width-module.slang-module
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -stage compute -entry computeMain -target cuda -r tests/diagnostics/vector-dot-oversized-width-module.slang-module

// The direct-compile case is covered by vector-dot-oversized-width.slang. This one covers the path
// that could actually hide a truncated `dot`: an oversized vector reaches a *serialized module*
// without the width error firing (compiling the module alone reports nothing), so the guarantee that
// the bound never truncates a real result rests on the width being rejected when the module is
// consumed. The call's result has to be used, or dead-code elimination removes it and the test
// passes without exercising anything.

import vector_dot_oversized_width_module;

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    vector<float, 5000> a = (vector<float, 5000>)1.0f;
    outputBuffer[0] = wideDot(a, a);
//CHECK:                     ^ see call to 'wideDot'
}

//CHECK: static assertion failed
//CHECK: static assertion failed, dot is only implemented for vectors of up to 4 elements
//CHECK: see call to 'dot'
//CHECK: invalid vector element count
//CHECK: vector has invalid element count '5000', valid values are between '1' and '4' inclusive
