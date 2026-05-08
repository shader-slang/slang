// Test that half-typed varying parameters legalize correctly for
// SPIRV. Slang's varying-param legalization has a sub-word (half /
// int16 / uint16) code path that's distinct from the int / float
// path; this test exercises a vertex / fragment varying chain
// using half values to ensure the SPIRV legalization keeps the
// 16-bit type intact (or correctly widens via the documented
// rules) instead of crashing or silently dropping.

//TEST:SIMPLE(filecheck=CHECK_VERT): -target spirv-asm -entry vertMain -stage vertex -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK_FRAG): -target spirv-asm -entry fragMain -stage fragment -emit-spirv-directly

struct VOut
{
    half3 color : COLOR;
    float4 position : SV_Position;
};

[shader("vertex")]
VOut vertMain(float3 pos : POSITION, half3 inColor : COLOR)
{
    VOut o;
    o.color = inColor;
    o.position = float4(pos, 1.0);
    return o;
}

struct FOut
{
    float4 color : SV_Target0;
};

[shader("fragment")]
FOut fragMain(half3 c : COLOR)
{
    FOut o;
    o.color = float4(float3(c), 1.0);
    return o;
}

// SPIRV must declare the Float16 capability and use the OpTypeFloat
// 16 instruction for the half-typed varying.
// CHECK_VERT: OpEntryPoint Vertex
// CHECK_VERT: OpTypeFloat 16

// Same on the fragment side — the half varying input must round-
// trip as a 16-bit float type.
// CHECK_FRAG: OpEntryPoint Fragment
// CHECK_FRAG: OpTypeFloat 16
