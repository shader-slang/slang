// Tests that bitfieldInsert/bitfieldExtract do not emit uint32_t in HLSL output.
// uint32_t is not available in HLSL versions prior to 2018 (e.g. fxc, or dxc with -HV 2016/2017).
// https://github.com/shader-slang/slang/issues/10294

//TEST:SIMPLE(filecheck=CHECK_INSERT):-target hlsl -entry computeInsert -stage compute
//TEST:SIMPLE(filecheck=CHECK_EXTRACT):-target hlsl -entry computeExtract -stage compute

RWStructuredBuffer<uint> outputBuffer;

// CHECK_INSERT: void computeInsert
// CHECK_INSERT-NOT: uint32_t

[numthreads(1, 1, 1)]
void computeInsert(uint3 tid : SV_DispatchThreadID)
{
    uint a = outputBuffer[0];
    uint b = outputBuffer[1];
    uint c = outputBuffer[2];
    uint d = outputBuffer[3];
    outputBuffer[0] = bitfieldInsert(a, b, c, d);
}

// CHECK_EXTRACT: void computeExtract
// CHECK_EXTRACT-NOT: uint32_t

[numthreads(1, 1, 1)]
void computeExtract(uint3 tid : SV_DispatchThreadID)
{
    uint a = outputBuffer[0];
    uint c = outputBuffer[2];
    uint d = outputBuffer[3];
    outputBuffer[0] = bitfieldExtract(a, c, d);
}
