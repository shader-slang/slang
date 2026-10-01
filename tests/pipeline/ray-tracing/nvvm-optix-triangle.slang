//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=UINT):-cuda -rt -output-using-type -compute-dispatch 8,1,1 -capability cuda_sm_8_0 -Xslang -DFLOAT_PAYLOAD=0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=UINT):-cuda -rt -output-using-type -compute-dispatch 8,1,1 -capability cuda_sm_8_0 -Xslang -DFLOAT_PAYLOAD=0 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=FLOAT):-cuda -rt -output-using-type -compute-dispatch 8,1,1 -capability cuda_sm_8_0 -Xslang -DFLOAT_PAYLOAD=1 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=FLOAT):-cuda -rt -output-using-type -compute-dispatch 8,1,1 -capability cuda_sm_8_0 -Xslang -DFLOAT_PAYLOAD=1 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=UINT):-cuda -rt -output-using-type -compute-dispatch 8,1,1 -capability cuda_sm_8_0 -Xslang -DFLOAT_PAYLOAD=0 -Xslang -O3
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=FLOAT):-cuda -rt -output-using-type -compute-dispatch 8,1,1 -capability cuda_sm_8_0 -Xslang -DFLOAT_PAYLOAD=1 -Xslang -O3

//TEST_INPUT: set scene = AccelerationStructure
RaytracingAccelerationStructure scene;
//TEST_INPUT: ubuffer(data=[3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559 3735928559],stride=4):out,name=output
RWStructuredBuffer<uint> output;

#ifndef FLOAT_PAYLOAD
#define FLOAT_PAYLOAD 0
#endif
#if FLOAT_PAYLOAD
struct Payload { float4 value; };
#else
struct Payload { uint value; };
#endif

[shader("raygeneration")]
void raygenMain()
{
    uint x = DispatchRaysIndex().x;
    RayDesc ray;
    ray.Origin = (x & 1) == 0 ? float3(0.25, 0.125, 1) : float3(2, 2, 1);
    ray.Direction = float3(0, 0, -1);
    ray.TMin = 0;
    ray.TMax = 10;
    Payload payload;
#if FLOAT_PAYLOAD
    payload.value = float4(100 + x, 200 + x, 300 + x, 400 + x);
#else
    payload.value = 100 + x;
#endif
    TraceRay(scene, RAY_FLAG_NONE, 255, 0, 1, 0, ray, payload);
#if FLOAT_PAYLOAD
    output[4 * x] = asuint(payload.value.x);
    output[4 * x + 1] = asuint(payload.value.y);
    output[4 * x + 2] = asuint(payload.value.z);
    output[4 * x + 3] = asuint(payload.value.w);
#else
    output[x] = payload.value;
#endif
}

[shader("miss")]
void missMain(inout Payload payload)
{
#if FLOAT_PAYLOAD
    payload.value += float4(2000, 2100, 2200, 2300);
#else
    payload.value += 2000;
#endif
}

[shader("closesthit")]
void closestHitMain(inout Payload payload, in BuiltInTriangleIntersectionAttributes attributes)
{
#if FLOAT_PAYLOAD
    payload.value += float4(1000 + attributes.barycentrics.x,
        1100 + attributes.barycentrics.y, 1200, 1300);
#else
    payload.value += 1000;
#endif
}

// UINT: type: uint32_t
// UINT-NEXT: {{^}}1100{{$}}
// UINT-NEXT: {{^}}2101{{$}}
// UINT-NEXT: {{^}}1102{{$}}
// UINT-NEXT: {{^}}2103{{$}}
// UINT-NEXT: {{^}}1104{{$}}
// UINT-NEXT: {{^}}2105{{$}}
// UINT-NEXT: {{^}}1106{{$}}
// UINT-NEXT: {{^}}2107{{$}}
// UINT-NEXT: {{^}}3735928559{{$}}
// UINT-NEXT: {{^}}3735928559{{$}}
// UINT-NEXT: {{^}}3735928559{{$}}
// UINT-NEXT: {{^}}3735928559{{$}}
// UINT-NEXT: {{^}}3735928559{{$}}
// UINT-NEXT: {{^}}3735928559{{$}}
// UINT-NEXT: {{^}}3735928559{{$}}
// UINT-NEXT: {{^}}3735928559{{$}}
// UINT-NEXT: {{^}}3735928559{{$}}
// UINT-NEXT: {{^}}3735928559{{$}}
// UINT-NEXT: {{^}}3735928559{{$}}
// UINT-NEXT: {{^}}3735928559{{$}}
// UINT-NEXT: {{^}}3735928559{{$}}
// UINT-NEXT: {{^}}3735928559{{$}}
// UINT-NEXT: {{^}}3735928559{{$}}
// UINT-NEXT: {{^}}3735928559{{$}}
// UINT-NEXT: {{^}}3735928559{{$}}
// UINT-NEXT: {{^}}3735928559{{$}}
// UINT-NEXT: {{^}}3735928559{{$}}
// UINT-NEXT: {{^}}3735928559{{$}}
// UINT-NEXT: {{^}}3735928559{{$}}
// UINT-NEXT: {{^}}3735928559{{$}}
// UINT-NEXT: {{^}}3735928559{{$}}
// UINT-NEXT: {{^}}3735928559{{$}}
// UINT-NEXT: {{^}}3735928559{{$}}
// UINT-NEXT: {{^}}3735928559{{$}}

// FLOAT: type: uint32_t
// FLOAT-NEXT: {{^}}1149862912{{$}}
// FLOAT-NEXT: {{^}}1151502336{{$}}
// FLOAT-NEXT: {{^}}1153138688{{$}}
// FLOAT-NEXT: {{^}}1154777088{{$}}
// FLOAT-NEXT: {{^}}1157844992{{$}}
// FLOAT-NEXT: {{^}}1158664192{{$}}
// FLOAT-NEXT: {{^}}1159483392{{$}}
// FLOAT-NEXT: {{^}}1160302592{{$}}
// FLOAT-NEXT: {{^}}1149879296{{$}}
// FLOAT-NEXT: {{^}}1151518720{{$}}
// FLOAT-NEXT: {{^}}1153155072{{$}}
// FLOAT-NEXT: {{^}}1154793472{{$}}
// FLOAT-NEXT: {{^}}1157853184{{$}}
// FLOAT-NEXT: {{^}}1158672384{{$}}
// FLOAT-NEXT: {{^}}1159491584{{$}}
// FLOAT-NEXT: {{^}}1160310784{{$}}
// FLOAT-NEXT: {{^}}1149895680{{$}}
// FLOAT-NEXT: {{^}}1151535104{{$}}
// FLOAT-NEXT: {{^}}1153171456{{$}}
// FLOAT-NEXT: {{^}}1154809856{{$}}
// FLOAT-NEXT: {{^}}1157861376{{$}}
// FLOAT-NEXT: {{^}}1158680576{{$}}
// FLOAT-NEXT: {{^}}1159499776{{$}}
// FLOAT-NEXT: {{^}}1160318976{{$}}
// FLOAT-NEXT: {{^}}1149912064{{$}}
// FLOAT-NEXT: {{^}}1151551488{{$}}
// FLOAT-NEXT: {{^}}1153187840{{$}}
// FLOAT-NEXT: {{^}}1154826240{{$}}
// FLOAT-NEXT: {{^}}1157869568{{$}}
// FLOAT-NEXT: {{^}}1158688768{{$}}
// FLOAT-NEXT: {{^}}1159507968{{$}}
// FLOAT-NEXT: {{^}}1160327168{{$}}
// FLOAT-NEXT: {{^}}3735928559{{$}}
// FLOAT-NEXT: {{^}}3735928559{{$}}
