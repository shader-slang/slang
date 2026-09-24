// An escaping lane address cannot be replaced with a private whole-vector update.
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_8_0 -O0 -o -
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_8_0 -O3 -o -

RWStructuredBuffer<uint> outputBuffer;

[noinline]
void toggle(inout bool value)
{
    value = !value;
}

[numthreads(4, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    bool4 flags = bool4(true, false, true, false);
    toggle(flags[tid.x]);
    outputBuffer[tid.x] = uint(flags[tid.x]);
}
//CHECK: direct NVVM lowering does not support Slang IR instruction or shape 'sequential element pointer: Ptr<bool, addressSpace=2147483647, access=0, operands=4, layout=ScalarLayout>'
