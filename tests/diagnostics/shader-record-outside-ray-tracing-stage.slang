// On CUDA/OptiX a shader record is only available to ray tracing programs, so a compute kernel
// that reads one is rejected instead of calling `optixGetSbtDataPointer()` from a plain kernel.
// See shader-slang/slang#12628.

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target cuda -entry csMain -stage compute
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target ptx -entry csMain -stage compute

struct SbtData
{
    float4 color;
};

layout(shaderRecordEXT) ConstantBuffer<SbtData> gSbtRecordData;
//CHECK:                                        ^^^^^^^^^^^^^^ error E39033
//CHECK:                                        ^^^^^^^^^^^^^^ this shader record is used by entry point 'csMain', which is not a ray tracing stage

RWStructuredBuffer<float4> gOut;

// The read sits in a helper, so the kernel reaches the record only through a call.
float4 readColor()
{
    return gSbtRecordData.color;
}

[shader("compute")]
[numthreads(1, 1, 1)]
void csMain()
{
    gOut[0] = readColor();
}
