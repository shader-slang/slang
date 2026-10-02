// A resource element type alone does not carry its physical storage format. Surface legalization
// resolves typed accesses through entry/global binding selectors and declared field keys. Passing
// the resource through an ordinary helper loses that provenance, so preflight rejects it rather
// than guessing the physical format from `float`.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_7_0 -o nvvm-formatted-surface-provenance.ptx

[format("r16f")]
RWTexture2D<float> inputTexture;
RWStructuredBuffer<float> outputBuffer;

[noinline]
float readThroughHelper(RWTexture2D<float> resource, int2 coordinate)
{
    return resource[coordinate];
}

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    outputBuffer[0] = readThroughHelper(inputTexture, int2(dispatchThreadID.xy));
}

//CHECK: direct NVVM lowering does not support Slang IR instruction or shape 'imageLoad'
