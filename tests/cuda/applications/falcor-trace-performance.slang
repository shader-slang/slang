// SPDX-FileCopyrightText: The Khronos Group, Inc.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
// Runtime performance corpus: replay with issue-nvvm-backend/shader-performance.py.
// Compile-time parameters vary actual words independently of the pipeline capacity.
#ifndef PAYLOAD_WORDS
#define PAYLOAD_WORDS 1
#endif
#ifndef NESTED_TRACE
#define NESTED_TRACE 0
#endif
#ifndef DO_TRACE
#define DO_TRACE 1
#endif

struct Payload
{
    uint words[PAYLOAD_WORDS];
};

RaytracingAccelerationStructure scene;
RWStructuredBuffer<uint> output;
uint size;
uint seed;
uint traceCount;

[shader("miss")]
void miss(inout Payload payload)
{
    [ForceUnroll]
    for (uint w = 0; w < PAYLOAD_WORDS; ++w)
        payload.words[w] += 7;
}

struct VisibilityPayload
{
    uint value;
};

[shader("miss")]
void visibilityMiss(inout VisibilityPayload payload)
{
    payload.value = 11;
}

[shader("closesthit")]
void closestHit(inout Payload payload, BuiltInTriangleIntersectionAttributes attributes)
{
    [ForceUnroll]
    for (uint w = 0; w < PAYLOAD_WORDS; ++w)
        payload.words[w] += 3;
#if NESTED_TRACE
    VisibilityPayload visibility = {};
    RayDesc ray;
    ray.Origin = float3(0, 0, 1);
    ray.Direction = float3(0, 0, -1);
    ray.TMin = 0;
    ray.TMax = 2;
    TraceRay(
        scene,
        RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER,
        0,
        0,
        0,
        1,
        ray,
        visibility);
    [ForceUnroll]
    for (uint w = 0; w < PAYLOAD_WORDS; ++w)
        payload.words[w] += visibility.value;
#endif
}

[shader("raygeneration")]
void raygen()
{
    uint2 pixel = DispatchRaysIndex().xy;
    uint index = pixel.y * size + pixel.x;
    uint value = index + seed;
#if DO_TRACE
    Payload payload;
    [ForceUnroll]
    for (uint w = 0; w < PAYLOAD_WORDS; ++w)
        payload.words[w] = value + w;
    RayDesc ray;
    ray.Origin = float3((float2(pixel) + 0.5) / float(size) * 2 - 1, 1);
    // Alternate guaranteed hits and misses, avoiding triangle boundary ambiguity.
    ray.Origin.x += ((index & 1) != 0) ? 100.0 : 0.0;
    ray.Direction = float3(0, 0, -1);
    ray.TMin = 0;
    ray.TMax = 2;
    for (uint t = 0; t < traceCount; ++t)
        TraceRay(scene, RAY_FLAG_FORCE_OPAQUE, 0xff, 0, 0, 0, ray, payload);
    value = 0;
    [ForceUnroll]
    for (uint w = 0; w < PAYLOAD_WORDS; ++w)
        value += payload.words[w];
#endif
    output[index] = value;
}
