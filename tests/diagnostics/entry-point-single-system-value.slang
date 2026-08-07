// Positive control for the duplicate-system-value diagnostic (E30706, #6319): binding a system
// value once, binding the same base name at DISTINCT indices, or binding SV_Barycentrics in both
// its perspective and no-perspective forms must all still compile. This guards against the
// uniqueness check (added in validateEntryPoint) firing on legal code.
//
// Uses the FileCheck runner rather than DIAGNOSTIC_TEST so the negation has real force: under
// DIAGNOSTIC_TEST a `//CHECK-NOT:` line is parsed as an ordinary comment (the annotation parser
// only recognizes exactly `//<prefix>:`), so it would assert nothing. Here `CHECK: OpEntryPoint`
// additionally proves real SPIR-V was produced — impossible if E30706 had aborted compilation.

//TEST:SIMPLE(filecheck=SINGLE):-target spirv-asm -entry singleVertex -stage vertex
//TEST:SIMPLE(filecheck=DISTINCT):-target spirv-asm -entry distinctTargets -stage fragment
//TEST:SIMPLE(filecheck=BARY):-target spirv-asm -entry splitBarycentrics -stage fragment -profile glsl_450+GL_EXT_fragment_shader_barycentric
//TEST:SIMPLE(filecheck=BARYFIELD):-target spirv-asm -entry splitBarycentricsInStruct -stage fragment -profile glsl_450+GL_EXT_fragment_shader_barycentric
//TEST:SIMPLE(filecheck=CLIPIDX):-target glsl -entry distinctClipDistances -stage vertex

// A single input system value is fine.
float4 singleVertex(uint a : SV_VertexID) : SV_Position
{
    return float4(float(a), 0, 0, 1);
}
//SINGLE-NOT: E30706
//SINGLE: OpEntryPoint

// The same base name at distinct indices binds distinct render targets and must stay clean.
void distinctTargets(out float4 a : SV_Target0, out float4 b : SV_Target1)
{
    a = float4(1, 0, 0, 1);
    b = float4(0, 1, 0, 1);
}
//DISTINCT-NOT: E30706
//DISTINCT: OpEntryPoint

// `noperspective` selects a different builtin for SV_Barycentrics, so the perspective and
// no-perspective coordinates are two separate bindings that may both be read by one fragment
// shader. The BaryCoord/BaryCoordNoPersp pair below proves they really did stay distinct through
// emit, so this pins more than the absence of the diagnostic.
void splitBarycentrics(
    float3 bary : SV_Barycentrics,
    noperspective float3 baryNoPerspective : SV_Barycentrics,
    out float4 result : SV_Target)
{
    result = float4(bary + baryNoPerspective, 1);
}
//BARY-NOT: E30706
//BARY-DAG: BuiltIn BaryCoordKHR
//BARY-DAG: BuiltIn BaryCoordNoPerspKHR

struct BaryInput
{
    float3 bary : SV_Barycentrics;
    noperspective float3 baryNoPerspective : SV_Barycentrics;
}

// The same split reached through an aggregate: the interpolation modifier sits on the struct field
// rather than the parameter, which is the decl the collection walk sees in this shape.
void splitBarycentricsInStruct(BaryInput input, out float4 result : SV_Target)
{
    result = float4(input.bary + input.baryNoPerspective, 1);
}
//BARYFIELD-NOT: E30706
//BARYFIELD-DAG: BuiltIn BaryCoordKHR
//BARYFIELD-DAG: BuiltIn BaryCoordNoPerspKHR

struct ClipOut
{
    float4 pos : SV_Position;
    float c0 : SV_ClipDistance0;
    float c1 : SV_ClipDistance1;
}

// SV_ClipDistance is one of the system values whose index does select a distinct binding: the GLSL
// legalization pass turns the suffix into an array index, so these two reach gl_ClipDistance[0] and
// gl_ClipDistance[1] and must stay clean. The indexed accesses below are what prove the index
// survived. (SV_CullDistance looks symmetric but is not — see dupCullDistance in
// entry-point-duplicate-system-value.slang.)
ClipOut distinctClipDistances()
{
    ClipOut o;
    o.pos = float4(0, 0, 0, 1);
    o.c0 = 1;
    o.c1 = 2;
    return o;
}
//CLIPIDX-NOT: E30706
//CLIPIDX-DAG: gl_ClipDistance[0]
//CLIPIDX-DAG: gl_ClipDistance[1]
