//TEST:SIMPLE(filecheck=CHECK): -allow-glsl -target spirv-asm

RWStructuredBuffer<uint> result;

// Test GLSL uniform block with scalar layout qualifier
// CHECK: SLANG_ParameterGroup_Data_uniform_natural
// CHECK: OpMemberDecorate %SLANG_ParameterGroup_Data_uniform_natural 0 Offset 0
// CHECK: OpMemberDecorate %SLANG_ParameterGroup_Data_uniform_natural 1 Offset 4
layout(scalar) uniform Data_uniform {
    uint a;
    uint3 b;
};

// Test HLSL cbuffer with scalar layout qualifier  
// CHECK: SLANG_ParameterGroup_Data_cbuffer_natural
// CHECK: OpMemberDecorate %SLANG_ParameterGroup_Data_cbuffer_natural 0 Offset 0
// CHECK: OpMemberDecorate %SLANG_ParameterGroup_Data_cbuffer_natural 1 Offset 4
layout(scalar) cbuffer Data_cbuffer {
    uint c;
    uint3 d;
};

// Reference: ScalarDataLayout should work correctly
struct Data {
    uint e;
    uint3 f;
};
ConstantBuffer<Data, ScalarDataLayout> Data_ConstantBufferScalarDataLayout;

[shader("compute")]
[numthreads(1,1,1)]
void computeMain(uint3 threadId : SV_DispatchThreadID)
{
    uint index = threadId.x;
    result[index] = 
        a + b.x +
        c + d.y +
        Data_ConstantBufferScalarDataLayout.e + Data_ConstantBufferScalarDataLayout.f.z;
}