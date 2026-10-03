// SPDX-FileCopyrightText: Copyright (c) 2026 NVIDIA CORPORATION & AFFILIATES. All rights reserved.
// SPDX-License-Identifier: Apache-2.0

//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 4,1,1 -capability cuda_sm_8_0 -Xslang -O3
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 4,1,1 -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 4,1,1 -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3

//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvrtc -entry raygenMain -stage raygeneration -DCHUNK_COUNT=8 -capability cuda_sm_8_0 -O3 -o -
//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvvm -entry raygenMain -stage raygeneration -DCHUNK_COUNT=8 -capability cuda_sm_8_0 -O0 -o -
//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvvm -entry raygenMain -stage raygeneration -DCHUNK_COUNT=8 -capability cuda_sm_8_0 -O3 -o -

// Reduced from Falcor2 reference_pathtracer.slang at revision 4ab8df487e479c78dfc30f064ad05ab33a609ba4.
// PathPayload contains PathState plus an empty OpacityEvaluator. Empty-type legalization keeps
// the public evaluator field as Void so field indices remain stable. NVVM payload admission must
// agree with the existing serializer that this field occupies no storage. Nested empty fields at
// the beginning, middle and end extend that shape. Runtime tests check four live words through
// hit/miss stages within the harness limit; compile tests check the 32-word payload boundary.
#ifndef CHUNK_COUNT
#define CHUNK_COUNT 1
#endif

public struct EmptyEvaluator {}

public struct Chunk
{
    public EmptyEvaluator before;
    public uint2 low;
    public EmptyEvaluator middle;
    public float2 high;
    public EmptyEvaluator after;
}

public struct PathState
{
    public EmptyEvaluator before;
    public Chunk chunks[CHUNK_COUNT];
}

[raypayload]
public struct PathPayload
{
    public PathState path : read(caller, closesthit, miss) : write(caller, closesthit, miss);
    public EmptyEvaluator opacityEvaluator : read(anyhit) : write(caller);
}

//TEST_INPUT: set scene = AccelerationStructure
RaytracingAccelerationStructure scene;
//TEST_INPUT: ubuffer(data=[3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559],stride=4):out,name=output
RWStructuredBuffer<uint> output;

bool matches(PathPayload payload, uint x, uint delta)
{
    bool ok = true;
    for (uint i = 0; i < CHUNK_COUNT; ++i)
    {
        uint base = x * 100 + i * 4 + delta;
        ok &= all(payload.path.chunks[i].low == uint2(base, base + 1));
        ok &= all(payload.path.chunks[i].high == float2(base + 2, base + 3));
    }
    return ok;
}

void updatePayload(inout PathPayload payload, uint delta)
{
    uint x = DispatchRaysIndex().x;
    bool inputOK = matches(payload, x, 0);
    for (uint i = 0; i < CHUNK_COUNT; ++i)
    {
        payload.path.chunks[i].low += uint2(delta);
        payload.path.chunks[i].high += float2(delta);
    }
    if (!inputOK)
        payload.path.chunks[0].low.x = 0xFFFFFFFFu;
}

[shader("raygeneration")]
void raygenMain()
{
    uint x = DispatchRaysIndex().x;
    PathPayload payload = {};
    for (uint i = 0; i < CHUNK_COUNT; ++i)
    {
        uint base = x * 100 + i * 4;
        payload.path.chunks[i].low = uint2(base, base + 1);
        payload.path.chunks[i].high = float2(base + 2, base + 3);
    }
    RayDesc ray;
    ray.Origin = (x & 1) == 0 ? float3(0.25, 0.125, 1) : float3(2, 2, 1);
    ray.Direction = float3(0, 0, -1);
    ray.TMin = 0;
    ray.TMax = 10;
    TraceRay(scene, RAY_FLAG_NONE, 255, 0, 1, 0, ray, payload);
    uint delta = (x & 1) == 0 ? 1000 : 2000;
    output[x] = matches(payload, x, delta) ? 6000 + x : 0;
}

[shader("miss")]
void missMain(inout PathPayload payload)
{
    updatePayload(payload, 2000);
}

[shader("closesthit")]
void closestHitMain(inout PathPayload payload, in BuiltInTriangleIntersectionAttributes attributes)
{
    updatePayload(payload, 1000);
}

// CHECK: type: uint32_t
// CHECK-NEXT: {{^}}6000{{$}}
// CHECK-NEXT: {{^}}6001{{$}}
// CHECK-NEXT: {{^}}6002{{$}}
// CHECK-NEXT: {{^}}6003{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}

// PTX: .visible .entry
