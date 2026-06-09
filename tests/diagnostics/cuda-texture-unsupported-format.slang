//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK):-target cuda -entry computeMain -stage compute

[format("rgb10_a2")]
RWTexture2D<float4> tex;
//CHECK: texture format 'rgb10_a2' is not supported for CUDA surface read/write operations
//CHECK: unsupported CUDA texture format

RWStructuredBuffer<float> outputBuffer;

[numthreads(1, 1, 1)]
void computeMain()
{
    float4 v = tex.Load(int2(0, 0));
    outputBuffer[0] = v.x;
}
