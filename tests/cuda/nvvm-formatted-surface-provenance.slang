// A formatted resource's element type does not carry its physical storage format. The direct
// backend deliberately resolves `[format]` only when the CUDA-prelude surface helper receives a
// canonical load from the collected global field. Passing the resource through another helper
// loses that provenance, so preflight must reject the shape instead of guessing from `float`.

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

//CHECK: direct NVVM lowering does not support Slang IR instruction or shape 'GenericAsm'
