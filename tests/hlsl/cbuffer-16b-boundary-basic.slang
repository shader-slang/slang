//TEST:SIMPLE(filecheck=CHECK):-profile sm_6_6 -stage compute -entry Main -target spirv -fvk-use-dx-layout

// DXC arguments: dxc cbuffer-16b-boundary-basic.slang -T cs_6_6 -E Main -spirv -fvk-use-dx-layout -Fo test.dxc.spv --enable-16bit-types

struct Constants
{
    // CHECK: OpMemberDecorate {{.*}} 0 Offset 0
    float3 Position;

    // CHECK: OpMemberDecorate {{.*}} 1 Offset 16
    float3 Forward;

    // CHECK: OpMemberDecorate {{.*}} 2 Offset 32
    float3 Right;

    // CHECK: OpMemberDecorate {{.*}} 3 Offset 44
    float PackedAfterRight;

    // CHECK: OpMemberDecorate {{.*}} 4 Offset 48
    float PackedBeforeUp;

    // CHECK: OpMemberDecorate {{.*}} 5 Offset 52
    float3 Up;

    // CHECK: OpMemberDecorate {{.*}} 6 Offset 64
    float NearPlane;

    // CHECK: OpMemberDecorate {{.*}} 7 Offset 68
    float FarPlane;

    // CHECK: OpMemberDecorate {{.*}} 8 Offset 72
    vector<float16_t, 3> Half3Test;

    // CHECK: OpMemberDecorate {{.*}} 9 Offset 80
    float PackedAfterHalf3Test;

    // CHECK: OpMemberDecorate {{.*}} 10 Offset 84
    vector<float16_t, 2> HalfVec2;

    // CHECK: OpMemberDecorate {{.*}} 11 Offset 96
    float16_t HalfArray[2];

    // CHECK: OpMemberDecorate {{.*}} 12 Offset 116
    float Fov;

    // CHECK: OpMemberDecorate {{.*}} 13 Offset 128
    float2x3 Mat1;

    // CHECK: OpMemberDecorate {{.*}} 14 Offset 168
    float2 Extra7;

    // CHECK: OpMemberDecorate {{.*}} 15 Offset 176
    float16_t ht;

    // CHECK: OpMemberDecorate {{.*}} 16 Offset 184
    float64_t dt;
};


[[vk::binding(0, 0)]]
ConstantBuffer<Constants> constants : register(b0, space0);

[[vk::binding(1, 0)]]
RWTexture2D<float4> output : register(u1, space0);

[numthreads(32, 32, 1)]
void Main(uint3 id: SV_DispatchThreadID)
{
    output[id.xy] = normalize(float4(constants.Forward, 1.0f));
}

