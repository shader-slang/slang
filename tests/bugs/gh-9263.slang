//TEST:SIMPLE(filecheck=CHECK): -target spirv -profile spirv_1_6 -O3 -entry computeMain

// Test for issue #9263: Internal Compiler Error on ConstantBuffer<matrix> access

[[vk::binding(0, 0)]] RWTexture2D<float4> tex_out;
[[vk::binding(1, 0)]] ConstantBuffer<float4x4> mat_camera;

[shader("compute")]
[numthreads(8, 8, 1)]
void computeMain(uint3 thread_i: SV_DispatchThreadID) {
    float4 vec;

    // Test vector subscript
    vec = mat_camera[3];

    // Test scalar subscript
    vec.x = mat_camera[3][0];

    // Test member access
    vec.x = mat_camera._41;

    // Test copy to standalone float4x4 before access
    float4x4 mat_copy = mat_camera;
    vec = mat_copy[3];
    vec.x = mat_copy[3][0];
    vec.x = mat_copy._41;

    // Writing output so it doesn't get optimized away
    tex_out[thread_i.xy] = vec;
}

// CHECK: OpCapability
