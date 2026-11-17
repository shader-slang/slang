//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry computeMain -stage compute
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-wgpu -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-vk -compute -shaderobj -output-using-type
//TEST(compute):COMPARE_COMPUTE_EX(filecheck-buffer=BUF):-cpu -compute -shaderobj -output-using-type

// Negating a float vector must flip the sign of a zero component, like scalar negation and like
// the other targets: -(+0.0) is -0.0. WGSL has a native unary '-' for f32, f16 and i32 vectors,
// so the emitter uses it. "vecN<f32>(0) - x" would give +0.0 for a +0.0 component (#14179).
// Unsigned vectors have no unary '-' in WGSL and keep the subtraction.

//TEST_INPUT:ubuffer(data=[0.0 0.0 0.0 1.5], stride=4):name=inputs
RWStructuredBuffer<float> inputs;

//TEST_INPUT:ubuffer(data=[0 0 0 0 0 0 0 0], stride=4):out,name=outputs
RWStructuredBuffer<uint> outputs;

// CHECK-NOT: vec3<f32>(0) -
// CHECK-NOT: vec2<i32>(0) -
// CHECK: vec2<u32>(0) -

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    float3 v = float3(inputs[0], inputs[1], inputs[2] + inputs[3]);
    float3 n = -v;
    float s = -inputs[0];

    int2 iv = int2(int(tid.x) + 5, int(inputs[3]));
    int2 ni = -iv;

    uint2 uv = uint2(tid.x + 1, tid.x + 2);
    uint2 nu = -uv;

    outputs[0] = asuint(n.x);
    outputs[1] = asuint(n.y);
    outputs[2] = asuint(n.z);
    outputs[3] = asuint(s);
    outputs[4] = uint(ni.x);
    outputs[5] = uint(ni.y);
    outputs[6] = nu.x;
    outputs[7] = nu.y;
}

// BUF: 2147483648
// BUF-NEXT: 2147483648
// BUF-NEXT: 3217031168
// BUF-NEXT: 2147483648
// BUF-NEXT: 4294967291
// BUF-NEXT: 4294967295
// BUF-NEXT: 4294967295
// BUF-NEXT: 4294967294
