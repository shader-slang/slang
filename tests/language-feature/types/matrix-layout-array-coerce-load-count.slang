// Regression for shader-slang/slang#13376: passing a cbuffer array of `row_major` matrices to a
// plain array parameter does not copy the array. The callee is specialized to read the one matrix
// it indexes directly from the cbuffer, as it was before #12992, which loads four rows. The DXIL
// lines ask for optimization because the test runner otherwise passes `-O0`.

//TEST:SIMPLE(filecheck=DXIL):-O2 -target dxil-asm -profile vs_6_0 -entry main -stage vertex -matrix-layout-row-major
//TEST:SIMPLE(filecheck=DXIL):-O2 -target dxil-asm -profile vs_6_0 -entry main -stage vertex -matrix-layout-column-major
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage vertex -matrix-layout-row-major
//TEST:SIMPLE(filecheck=HLSL):-target hlsl -entry main -stage vertex -matrix-layout-column-major
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry main -stage vertex -matrix-layout-row-major
//TEST:SIMPLE(filecheck=SPV):-target spirv-asm -entry main -stage vertex -matrix-layout-column-major

cbuffer Skin
{
    row_major float4x4 bones[64];
}

float3 skin(float4x4 b[64], uint i, float3 p)
{
    return mul(float4(p, 1), b[i]).xyz;
}

[shader("vertex")]
float4 main(float3 p : POSITION, uint i : BLENDINDICES) : SV_Position
{
    return float4(skin(bones, i, p), 1);
}

// DXIL: define void @main()
// DXIL-NOT: alloca
// DXIL-COUNT-4: call {{.*}} @dx.op.cbufferLoadLegacy
// DXIL-NOT: call {{.*}} @dx.op.cbufferLoadLegacy
// DXIL: ret void

// HLSL: float3 skin_0(uint
// HLSL: Skin_0.bones_0
// HLSL-NOT: float4x4 {{.*}}[int(64)]
// HLSL: float4 main(

// SPV-NOT: %_ptr_Function__arr_mat4v4float_int_64
// SPV: OpFunctionEnd
