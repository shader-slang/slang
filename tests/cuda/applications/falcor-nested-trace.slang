// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvrtc -entry raygenMain -stage raygeneration -capability cuda_sm_8_0 -O3 -o -
//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvvm -entry raygenMain -stage raygeneration -capability cuda_sm_8_0 -O0 -o -
//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvvm -entry raygenMain -stage raygeneration -capability cuda_sm_8_0 -O3 -o -
//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvrtc -entry closestHitMain -stage closesthit -capability cuda_sm_8_0 -O3 -o -
//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvvm -entry closestHitMain -stage closesthit -capability cuda_sm_8_0 -O0 -o -
//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvvm -entry closestHitMain -stage closesthit -capability cuda_sm_8_0 -O3 -o -
//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvrtc -entry missMain -stage miss -capability cuda_sm_8_0 -O3 -o -
//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvvm -entry missMain -stage miss -capability cuda_sm_8_0 -O0 -o -
//TEST:SIMPLE(filecheck=PTX): -target ptx -emit-cuda-via-nvvm -entry missMain -stage miss -capability cuda_sm_8_0 -O3 -o -

// Falcor traces a Boolean visibility ray while shading a closest hit. The outer payload must
// survive the nested call. Four dispatches cover outer hit/miss crossed with inner hit/miss.
// Execute with recursion depth2, two hit groups and two miss records; render-test fixes depth1.
RaytracingAccelerationStructure scene;
RWStructuredBuffer<uint> output;
struct PathPayload { uint words[32]; }

RayDesc makeRay(bool hit)
{
    return { hit ? float3(0.25, 0.125, 1) : float3(2, 2, 1),
        0, float3(0, 0, -1), 10 };
}

[shader("raygeneration")]
void raygenMain()
{
    uint x = DispatchRaysIndex().x;
    PathPayload p;
    for (uint i = 0; i < 32; ++i) p.words[i] = 100*x+i;
    TraceRay(scene, RAY_FLAG_NONE, 255, 0, 2, 0, makeRay((x&1)==0), p);
    bool ok = true;
    for (uint i = 0; i < 32; ++i)
        ok &= p.words[i] == 100*x+i+((x&1)==0 ? 1000 : 2000);
    output[x] = uint(ok);
}

void shade(inout PathPayload p, uint increment)
{
    uint x = DispatchRaysIndex().x;
    bool visible = (x&1)!=0;
    TraceRay(scene, RAY_FLAG_NONE, 255, 1, 2, 1, makeRay((x&2)==0), visible);
    output[x+4] = uint(visible == ((x&1)==0));
    for (uint i = 0; i < 32; ++i) p.words[i] += increment;
}

[shader("closesthit")]
void closestHitMain(inout PathPayload p, in BuiltInTriangleIntersectionAttributes attributes)
{
    shade(p, 1000);
}

[shader("miss")]
void missMain(inout PathPayload p)
{
    shade(p, 2000);
}

[shader("closesthit")]
void visibilityHit(inout bool visible, in BuiltInTriangleIntersectionAttributes attributes)
{
    uint x = DispatchRaysIndex().x;
    output[x+8] = uint(visible == ((x&1)!=0) && (x&2)==0);
    visible = !visible;
}

[shader("miss")]
void visibilityMiss(inout bool visible)
{
    uint x = DispatchRaysIndex().x;
    output[x+8] = uint(visible == ((x&1)!=0) && (x&2)!=0);
    visible = !visible;
}

// PTX: _optix_trace_typed_32
