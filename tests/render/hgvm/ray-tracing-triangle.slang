//DISABLE_TEST:INTERPRET(hgvm): -api d3d12
// HGVM does not yet implement the ray-tracing host operations exercised by this test.

import slang.pipeline_rt;

RWTexture2D<T> allocateTexture2D<T : ITexelElement>(uint width, uint height)
{
    uint one = 1;
    return hgbc_asm
    {
    result:
        $$RWTexture2D<T> = OpAllocateResource Texture Texture2D $__getHGBCNaturalTextureFormat<T>()
            DeviceLocal UnorderedAccess $$T $width $height $one 1 1 0 ShaderResource UnorderedAccess
                CopySource CopyDestination RenderTarget;
    };
}

void fail(String message)
{
    hgbc_asm
    {
        OpFail $message;
    };
}

[raypayload]
struct Payload
{
    float4 color : read(caller)
        : write(caller, closesthit, miss);
};

[shader("compute")]
[numthreads(1, 1, 1)]
void initializeGeometry(RWStructuredBuffer<float3> vertices, RWStructuredBuffer<uint> indices)
{
    vertices[0] = float3(-2.0, -2.0, 0.0);
    vertices[1] = float3(2.0, -2.0, 0.0);
    vertices[2] = float3(0.0, 2.0, 0.0);
    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;
}

[shader("raygeneration")]
void rayGeneration(
    uniform RaytracingAccelerationStructure scene,
    uniform RWTexture2D<float4> output)
{
    RayDesc ray;
    ray.Origin = float3(0.0, 0.0, -1.0);
    ray.Direction = float3(0.0, 0.0, 1.0);
    ray.TMin = 0.001;
    ray.TMax = 1000.0;

    Payload payload = { float4(0.0) };
    TraceRay(scene, RAY_FLAG_NONE, 0xff, 0, 0, 0, ray, payload);
    output[DispatchRaysIndex().xy] = payload.color;
}

[shader("closesthit")]
void closestHit(inout Payload payload, in BuiltInTriangleIntersectionAttributes attributes)
{
    payload.color = float4(1.0, attributes.barycentrics, 1.0);
}

[shader("miss")]
void miss(inout Payload payload)
{
    payload.color = float4(0.0, 0.0, 1.0, 1.0);
}

[shader("compute")]
[numthreads(1, 1, 1)]
void readPixel(RWTexture2D<float4> image, RWStructuredBuffer<float4> output)
{
    output[0] = image[uint2(0, 0)];
}

[host]
void main()
{
    RWStructuredBuffer<float3> vertices = RWStructuredBuffer<float3>::alloc(3);
    RWStructuredBuffer<uint> indices = RWStructuredBuffer<uint>::alloc(3);
    RWTexture2D<float4> image = allocateTexture2D<float4>(1, 1);
    RWStructuredBuffer<float4> output = RWStructuredBuffer<float4>::alloc(1);

    dispatch(initializeGeometry, uint3(1, 1, 1))(vertices, indices);
    RaytracingAccelerationStructure scene = createRaytracingScene(3, vertices, 3, indices);
    raytrace(rayGeneration, closestHit, miss)(uint3(1, 1, 1), scene, image);
    dispatch(readPixel, uint3(1, 1, 1))(image, output);

    float4 pixel = readBack(output, 0);
    if (pixel.x < 0.99 || pixel.w < 0.99)
    {
        if (pixel.z > 0.99 && pixel.w > 0.99)
            fail("HGVM ray tracing ran the miss shader for the test triangle");
        fail("HGVM ray tracing did not write the output texture");
    }
}
