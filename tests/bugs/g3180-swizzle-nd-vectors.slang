//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cpp -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target glsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target hlsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -emit-spirv-via-glsl -entry computeMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target spirv -emit-spirv-directly -entry computeMain -stage compute

// This test tests that assigning to a subscript of a unary swizzle works

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    vector<vector<int, 2>, 2> v;
//CHECK:                      ^ disallowed vector element type
//CHECK:                      ^ vector with disallowed element type 'vector<int,2>' encountered
    v.x[0] = 1;
    outputBuffer[dispatchThreadID.x] = v.x[0];
//CHECK:                                 ^ unsupported builtin type
//CHECK:                                 ^ 'vector<vector<int,2>,2>' is not a supported builtin type for the target.
}
