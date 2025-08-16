//TEST:SIMPLE(filecheck=SPIRV): -stage closesthit -entry chit -target spirv -fvk-use-c-layout
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

struct C {
    int i;
    A a;
};

struct D {
    C c;
    int j;
};

[[vk::shader_record]]
cbuffer ShaderRecord
{
    D d;
}

[[shader("closesthit")]]
void chit(inout RayPayload payload : SV_RayPayload, in float2 barys_yz : SV_IntersectionAttributes) {
    let barys = float3(1.0 - barys_yz.x - barys_yz.y, barys_yz);
    payload.color = barys;

    push.p->b = d.j + d.c.i;
}

// SPIRV: OpMemberDecorate %A_c 0 Offset 0
// SPIRV: OpMemberDecorate %A_c 1 Offset 8
// SPIRV: OpMemberDecorate %C_c 0 Offset 0
// SPIRV: OpMemberDecorate %C_c 1 Offset 8
// SPIRV: OpMemberDecorate %D_c 0 Offset 0
// SPIRV: OpMemberDecorate %D_c 1 Offset 24
// SPIRV: OpMemberDecorate %B_c 0 Offset 0
// SPIRV: OpMemberDecorate %B_c 1 Offset 16

// SPIRV: %_ptr_PushConstant_Push_c = OpTypePointer PushConstant %Push_c
// SPIRV: %_ptr_ShaderRecordBufferKHR_SLANG_ParameterGroup_ShaderRecord_c = OpTypePointer ShaderRecordBufferKHR %SLANG_ParameterGroup_ShaderRecord_c
