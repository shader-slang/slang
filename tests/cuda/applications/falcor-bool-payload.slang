// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception

//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 4,1,1 -capability cuda_sm_8_0 -Xslang -DFORM=0 -Xslang -O3
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 4,1,1 -capability cuda_sm_8_0 -Xslang -DFORM=0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 4,1,1 -capability cuda_sm_8_0 -Xslang -DFORM=0 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 4,1,1 -capability cuda_sm_8_0 -Xslang -DFORM=1 -Xslang -O3
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 4,1,1 -capability cuda_sm_8_0 -Xslang -DFORM=1 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 4,1,1 -capability cuda_sm_8_0 -Xslang -DFORM=1 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 4,1,1 -capability cuda_sm_8_0 -Xslang -DFORM=2 -Xslang -O3
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 4,1,1 -capability cuda_sm_8_0 -Xslang -DFORM=2 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 4,1,1 -capability cuda_sm_8_0 -Xslang -DFORM=2 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 4,1,1 -capability cuda_sm_8_0 -Xslang -DFORM=3 -Xslang -O3
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 4,1,1 -capability cuda_sm_8_0 -Xslang -DFORM=3 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 4,1,1 -capability cuda_sm_8_0 -Xslang -DFORM=3 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 4,1,1 -capability cuda_sm_8_0 -Xslang -DFORM=4 -Xslang -O3
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 4,1,1 -capability cuda_sm_8_0 -Xslang -DFORM=4 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 4,1,1 -capability cuda_sm_8_0 -Xslang -DFORM=4 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 4,1,1 -capability cuda_sm_8_0 -Xslang -DFORM=5 -Xslang -O3
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 4,1,1 -capability cuda_sm_8_0 -Xslang -DFORM=5 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 4,1,1 -capability cuda_sm_8_0 -Xslang -DFORM=5 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 4,1,1 -capability cuda_sm_8_0 -Xslang -DFORM=6 -Xslang -O3
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 4,1,1 -capability cuda_sm_8_0 -Xslang -DFORM=6 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 4,1,1 -capability cuda_sm_8_0 -Xslang -DFORM=6 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 4,1,1 -capability cuda_sm_8_0 -Xslang -DFORM=7 -Xslang -O3
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 4,1,1 -capability cuda_sm_8_0 -Xslang -DFORM=7 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 4,1,1 -capability cuda_sm_8_0 -Xslang -DFORM=7 -Xslang -emit-cuda-via-nvvm -Xslang -O3

//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 4,1,1 -capability cuda_sm_8_0 -Xslang -DFORM=1 -Xslang -DUSE_HIT_OBJECT=1 -Xslang -O3
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 4,1,1 -capability cuda_sm_8_0 -Xslang -DFORM=1 -Xslang -DUSE_HIT_OBJECT=1 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 4,1,1 -capability cuda_sm_8_0 -Xslang -DFORM=1 -Xslang -DUSE_HIT_OBJECT=1 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 4,1,1 -capability cuda_sm_8_0 -Xslang -DFORM=5 -Xslang -DUSE_HIT_OBJECT=1 -Xslang -O3
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 4,1,1 -capability cuda_sm_8_0 -Xslang -DFORM=5 -Xslang -DUSE_HIT_OBJECT=1 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 4,1,1 -capability cuda_sm_8_0 -Xslang -DFORM=5 -Xslang -DUSE_HIT_OBJECT=1 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 4,1,1 -capability cuda_sm_8_0 -Xslang -DFORM=6 -Xslang -DUSE_HIT_OBJECT=1 -Xslang -O3
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 4,1,1 -capability cuda_sm_8_0 -Xslang -DFORM=6 -Xslang -DUSE_HIT_OBJECT=1 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 4,1,1 -capability cuda_sm_8_0 -Xslang -DFORM=6 -Xslang -DUSE_HIT_OBJECT=1 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 4,1,1 -capability cuda_sm_8_0 -Xslang -DFORM=7 -Xslang -DUSE_HIT_OBJECT=1 -Xslang -O3
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 4,1,1 -capability cuda_sm_8_0 -Xslang -DFORM=7 -Xslang -DUSE_HIT_OBJECT=1 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 4,1,1 -capability cuda_sm_8_0 -Xslang -DFORM=7 -Xslang -DUSE_HIT_OBJECT=1 -Xslang -emit-cuda-via-nvvm -Xslang -O3

// Falcor VisibilityPayload contains a Boolean and a public empty evaluator. Other forms check
// byte sharing, partial final words and CUDA vector/matrix/record layout using the same traversal.
#ifndef USE_HIT_OBJECT
#define USE_HIT_OBJECT 0
#endif
#ifndef FORM
#define FORM 0
#endif
public struct EmptyEvaluator {}
#if FORM == 0
[raypayload]
public struct Payload
{
    public bool visible : read(caller, closesthit, miss, anyhit) : write(caller, closesthit, miss, anyhit);
    public EmptyEvaluator evaluator : read(anyhit) : write(caller);
}
#elif FORM == 1
typealias Payload = bool;
#elif FORM == 2
typealias Payload = bool2;
#elif FORM == 3
typealias Payload = bool3;
#elif FORM == 4
typealias Payload = bool4;
#elif FORM == 5
struct Payload { bool values[13]; };
#elif FORM == 6
typealias Payload = bool2x3;
#else
struct Payload
{
    bool first;
    uint number;
    bool3 middle;
    float last;
}
#endif

Payload makePayload(uint pattern)
{
    Payload p = {};
#if FORM == 0
    p.visible = (pattern & 1) != 0;
#elif FORM == 1
    p = (pattern & 1) != 0;
#elif FORM >= 2 && FORM <= 4
    for (uint i = 0; i < FORM; ++i) p[i] = ((pattern >> i) & 1) != 0;
#elif FORM == 5
    for (uint i = 0; i < 13; ++i) p.values[i] = ((pattern >> (i % 5)) & 1) != 0;
#elif FORM == 6
    for (uint r = 0; r < 2; ++r)
        for (uint c = 0; c < 3; ++c) p[r][c] = ((pattern >> ((3*r+c) % 5)) & 1) != 0;
#else
    p.first = (pattern & 1) != 0;
    p.number = pattern * 1234 + 0x12345678;
    p.middle = bool3((pattern & 2) != 0, (pattern & 4) != 0, (pattern & 8) != 0);
    p.last = float(pattern) + 0.5;
#endif
    return p;
}

bool matches(Payload p, uint pattern)
{
    Payload expected = makePayload(pattern);
#if FORM == 0
    return p.visible == expected.visible;
#elif FORM == 1
    return p == expected;
#elif FORM >= 2 && FORM <= 4
    return all(p == expected);
#elif FORM == 5
    bool ok = true;
    for (uint i = 0; i < 13; ++i) ok &= p.values[i] == expected.values[i];
    return ok;
#elif FORM == 6
    return all(p[0] == expected[0]) && all(p[1] == expected[1]);
#else
    return p.first == expected.first && p.number == expected.number &&
        all(p.middle == expected.middle) && p.last == expected.last;
#endif
}

//TEST_INPUT: set scene = AccelerationStructure
RaytracingAccelerationStructure scene;
//TEST_INPUT: ubuffer(data=[3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559],stride=4):out,name=output
RWStructuredBuffer<uint> output;

[shader("raygeneration")]
void raygenMain()
{
    uint x = DispatchRaysIndex().x;
    RayDesc ray = { (x & 1) == 0 ? float3(0.25, 0.125, 1) : float3(2, 2, 1),
        0, float3(0, 0, -1), 10 };
    Payload p = makePayload(x+1);
#if USE_HIT_OBJECT
    HitObject object = HitObject.TraceRay(scene, RAY_FLAG_NONE, 255, 0, 1, 0, ray, p);
    HitObject.Invoke(scene, object, p);
#else
    TraceRay(scene, RAY_FLAG_NONE, 255, 0, 1, 0, ray, p);
#endif
    output[x] = uint(matches(p, x + ((x & 1) == 0 ? 6 : 12)));
}

[shader("closesthit")]
void closestHitMain(inout Payload p, in BuiltInTriangleIntersectionAttributes attributes)
{
    uint x = DispatchRaysIndex().x;
    output[x+4] = uint(matches(p, x+1));
    p = makePayload(x+6);
}

[shader("miss")]
void missMain(inout Payload p)
{
    uint x = DispatchRaysIndex().x;
    output[x+4] = uint(matches(p, x+1));
    p = makePayload(x+12);
}

// CHECK: type: uint32_t
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}1{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}
