// An undecorated Float32 read-write texture uses native Float32 storage. The semantic Float32
// payload crosses the builder interface unchanged; the LLVM provider owns the i32 bitcast required
// by the NVVM surface intrinsic.

//TEST:SIMPLE(filecheck=CHECK): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0 -O3 -o -

RWTexture2D<float> inputTexture;
RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[0] = inputTexture[int2(dispatchThreadID.xy)];
}

//CHECK: suld.b.2d.b32.zero
