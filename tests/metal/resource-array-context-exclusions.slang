//TEST:SIMPLE(filecheck=METAL): -target metal -stage compute -entry computeMain

// Metal has no direct bound-argument form for nested or unsized resource arrays, or for buffer
// arrays. This optimization must not pretend that those value parameters have bound storage.
// Keep their existing context representation until their argument-buffer routing is implemented.

Texture2D<float4> nestedTextures[2][2];
Texture2D<float4> unsizedTextures[];
StructuredBuffer<float4> buffers[2];
RWStructuredBuffer<float4> output;

// METAL-LABEL: struct KernelContext
// METAL-DAG: array<array<texture2d<float, access::sample>, int(2)>, int(2)> nestedTextures_{{[0-9]+}};
// METAL-DAG: unsizedTextures_{{[0-9]+}}[];
// METAL-DAG: array<packed_float4 device*, int(2)> buffers_{{[0-9]+}};
// METAL: };

[noinline]
float4 sampleGlobal(uint index)
{
    return nestedTextures[index % 2][(index + 1) % 2].Load(int3(0)) +
        unsizedTextures[index].Load(int3(0)) + buffers[index % 2][index];
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    output[tid.x] = sampleGlobal(tid.x);
}
