//TEST:SIMPLE: -target ptx -stage raygeneration -entry raygenMain -emit-cuda-via-nvvm -capability cuda_sm_8_0 -O0 -o nvvm-optix-raygen.ptx.actual
//TEST:SIMPLE: -target ptx -stage raygeneration -entry raygenMain -emit-cuda-via-nvvm -capability cuda_sm_8_0 -O3 -o nvvm-optix-raygen.ptx.actual

// Runtime qualification changes both launch parameters and SBT data on the same pipeline.
cbuffer Globals
{
    RWStructuredBuffer<uint> output;
    uint base;
}

[shader("raygeneration")]
void raygenMain(uniform uint bias)
{
    uint3 index = DispatchRaysIndex();
    uint3 dimensions = DispatchRaysDimensions();
    uint linear = index.x + dimensions.x * (index.y + dimensions.y * index.z);
    uint first = linear * 7;
    output[first] = index.x;
    output[first + 1] = index.y;
    output[first + 2] = index.z;
    output[first + 3] = dimensions.x;
    output[first + 4] = dimensions.y;
    output[first + 5] = dimensions.z;
    output[first + 6] = base + bias + linear;
}
