// Required helper inlining retains the declared r16f binding at each call site.
//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_8_0 -O0 -o -
//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvvm -entry computeMain -stage compute -capability cuda_sm_8_0 -O3 -o -

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

// PTX: suld.b.2d.b16.zero
