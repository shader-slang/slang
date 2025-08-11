//TEST:SIMPLE(filecheck=SPIRV): -stage closesthit -entry chit -target spirv -fvk-use-cpu-layout
struct RayPayload
{
    float3 color;
};

struct A {
    int64_t a1;
    int32_t a2;
};

struct B {
    A a;
    uint32_t b;
};

struct Push {
    B *p;
};

[[vk::push_constant]] Push push;

[[shader("closesthit")]]
void chit(inout RayPayload payload : SV_RayPayload, in float2 barys_yz : SV_IntersectionAttributes) {
    let barys = float3(1.0 - barys_yz.x - barys_yz.y, barys_yz);
    payload.color = barys;

    push.p->b = 42;
}

// SPIRV: OpMemberDecorate %A_cpu 0 Offset 0
// SPIRV: OpMemberDecorate %A_cpu 1 Offset 8
// SPIRV: OpMemberDecorate %B_cpu 0 Offset 0
// SPIRV: OpMemberDecorate %B_cpu 1 Offset 16
// SPIRV: %_ptr_PushConstant_Push_cpu = OpTypePointer PushConstant %Push_cpu

