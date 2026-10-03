//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 1,1,1 -capability cuda_sm_8_0 -Xslang -O3
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 1,1,1 -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O0
//TEST(optix):COMPARE_COMPUTE_EX(filecheck-buffer=CHECK):-cuda -rt -output-using-type -compute-dispatch 1,1,1 -capability cuda_sm_8_0 -Xslang -emit-cuda-via-nvvm -Xslang -O3
//TEST:SIMPLE(filecheck=PTX): -o - -entry anyHitMain -stage anyhit -target ptx -capability cuda_sm_8_0 -emit-cuda-via-nvvm
//TEST:SIMPLE(filecheck=PTX): -o - -entry intersectionMain -stage intersection -target ptx -capability cuda_sm_8_0 -emit-cuda-via-nvvm
//TEST:SIMPLE(filecheck=PTX): -o - -entry anyHitMain -stage anyhit -target ptx -capability cuda_sm_8_0
//TEST:SIMPLE(filecheck=PTX): -o - -entry intersectionMain -stage intersection -target ptx -capability cuda_sm_8_0

//TEST_INPUT: set scene = AccelerationStructure
RaytracingAccelerationStructure scene;
//TEST_INPUT: ubuffer(data=[3735928559 3735928559],stride=4):out,name=output
RWStructuredBuffer<uint> output;

[shader("raygeneration")]
void raygenMain()
{
    uint x = DispatchRaysIndex().x;
    RayDesc ray = { float3(0.25 + 2 * x, 0.125, 1), 0, float3(0, 0, -1), 10 };
    uint payload = 123;
    TraceRay(scene, RAY_FLAG_NONE, 255, 0, 1, 0, ray, payload);
    output[x] = payload;
}

[shader("miss")]
void missMain(inout uint payload) { payload = 456; }

[shader("closesthit")]
void closestHitMain(inout uint payload, in BuiltInTriangleIntersectionAttributes attributes)
{
    payload = GeometryIndex();
}

[shader("anyhit")]
void anyHitMain(inout uint payload, in BuiltInTriangleIntersectionAttributes attributes)
{
    payload = GeometryIndex();
}

[shader("intersection")]
void intersectionMain()
{
    output[0] = GeometryIndex();
}

// CHECK: type: uint32_t
// CHECK-NEXT: {{^}}0{{$}}
// CHECK-NEXT: {{^}}3735928559{{$}}

// PTX: _optix_read_sbt_gas_idx
