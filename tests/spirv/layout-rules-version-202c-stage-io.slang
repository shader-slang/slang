// `-layout-rules-version 202c` changes buffer and pointer layouts only. Stage inputs and outputs
// have no memory layout, so their interface types are the same with or without the option: the
// matrix varyings below keep their natural-layout wrapper, and nothing uses the rounded rule.

//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -emit-spirv-directly -stage vertex -entry vmain -layout-rules-version 202c
//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -emit-spirv-directly -stage vertex -entry vmain -fvk-use-scalar-layout -layout-rules-version 202c

struct Inner
{
    float3 a;
    float b;
    int2 c;
};

struct VIn
{
    float4x4 m : M;
    Inner i : INNER;
};

struct VOut
{
    float4 pos : SV_Position;
    float4x4 m : TEXCOORD0;
    Inner i : TEXCOORD4;
};

VOut vmain(VIn vin)
{
    VOut o;
    o.pos = mul(vin.m, float4(1, 1, 1, 1));
    o.m = vin.m;
    o.i = vin.i;
    return o;
}

// CHECK-NOT: scalar_rounded
// CHECK: %_MatrixStorage_float4x4_ColMajor_logicalnatural = OpTypeStruct
// CHECK-NOT: scalar_rounded
